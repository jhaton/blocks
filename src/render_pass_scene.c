#include "render_pass_scene.h"
#include "helpers.h"
#include "math3d.h"
#include "mesh.h"
#include "renderer.h"
#include "shader.h"

typedef struct {
	SDL_GPUGraphicsPipeline* pipeline;
} scene_pass_state_t;

typedef struct {
	float view_projection[4][4];
	float model_matrix[4][4];
	float light_matrix[4][4];
} scene_vertex_uniforms_t;

typedef struct {
	float light_direction[4];
	float light_color_intensity[4];
	float camera_position_time[4];
	float albedo[4];
} scene_fragment_uniforms_t;

static bool scene_pass_init(renderer_t* renderer, render_pass_t* pass) {
	scene_pass_state_t* state = SDL_calloc(1, sizeof(*state));
	if (!state) {
		return false;
	}
	SDL_GPUShader* vertex = shader_library_load(&renderer->shaders, "scene.vert",
											 SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
	SDL_GPUShader* fragment = shader_library_load(&renderer->shaders, "scene.frag",
											   SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);
	if (!vertex || !fragment) {
		goto fail;
	}

	SDL_GPUVertexBufferDescription vertex_buffer = {
		.slot = 0,
		.pitch = sizeof(float) * 6,
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
	};
	SDL_GPUVertexAttribute attributes[2] = {
		{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0,
		},
		{
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = sizeof(float) * 3,
		},
	};
	SDL_GPUGraphicsPipelineCreateInfo info = {
		.vertex_shader = vertex,
		.fragment_shader = fragment,
		.target_info = {
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
				.format = renderer->scene_color_format,
			}},
			.has_depth_stencil_target = true,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		},
		.vertex_input_state = {
			.vertex_buffer_descriptions = &vertex_buffer,
			.num_vertex_buffers = 1,
			.vertex_attributes = attributes,
			.num_vertex_attributes = SDL_arraysize(attributes),
		},
		.depth_stencil_state = {
			.enable_depth_test = true,
			.enable_depth_write = true,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
		},
		.rasterizer_state = {
			.cull_mode = SDL_GPU_CULLMODE_BACK,
			.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
		},
	};
	state->pipeline = SDL_CreateGPUGraphicsPipeline(renderer->device, &info);
	if (!check_resource(state->pipeline, "create scene pipeline")) {
		goto fail;
	}

	SDL_ReleaseGPUShader(renderer->device, vertex);
	SDL_ReleaseGPUShader(renderer->device, fragment);
	pass->state = state;
	return true;

fail:
	if (vertex) {
		SDL_ReleaseGPUShader(renderer->device, vertex);
	}
	if (fragment) {
		SDL_ReleaseGPUShader(renderer->device, fragment);
	}
	SDL_free(state);
	return false;
}

static void scene_pass_resize(renderer_t* renderer, render_pass_t* pass, Uint32 width, Uint32 height) {
	(void)renderer;
	(void)pass;
	(void)width;
	(void)height;
}

static void scene_pass_execute(renderer_t* renderer, render_pass_t* pass, const frame_context_t* frame) {
	scene_pass_state_t* state = pass->state;
	const directional_light_component_t* light = scene_main_light(frame->scene);
	if (!light) {
		SDL_Log("Scene pass requires a main directional light");
		return;
	}

	SDL_GPUColorTargetInfo color = {
		.texture = renderer->scene_color_texture,
		.clear_color = (SDL_FColor){0.08f, 0.10f, 0.14f, 1.0f},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.cycle = true,
	};
	SDL_GPUDepthStencilTargetInfo depth = {
		.texture = renderer->scene_depth_texture,
		.clear_depth = 1.0f,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
		.cycle = true,
	};
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(frame->commands, &color, 1, &depth);
	if (!check_resource(render_pass, "begin scene render pass")) {
		return;
	}

	SDL_GPUTextureSamplerBinding shadow_binding = {
		.texture = renderer->shadow_depth_texture,
		.sampler = renderer->shadow_sampler,
	};
	SDL_BindGPUGraphicsPipeline(render_pass, state->pipeline);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &shadow_binding, 1);
	mesh_library_bind_box(&renderer->meshes, render_pass);

	for (entity_t entity = 0; entity < SCENE_MAX_ENTITIES; entity++) {
		if (!frame->scene->alive[entity] || !frame->scene->has_transform[entity] ||
			!frame->scene->has_renderable[entity]) {
			continue;
		}
		const transform_component_t* transform = &frame->scene->transforms[entity];
		if (!camera_test(frame->camera, transform->position[0] - transform->scale[0],
						 transform->position[1] - transform->scale[1],
						 transform->position[2] - transform->scale[2], transform->scale[0] * 2.0f,
						 transform->scale[1] * 2.0f, transform->scale[2] * 2.0f)) {
			continue;
		}
		const renderable_component_t* renderable = &frame->scene->renderables[entity];
		scene_vertex_uniforms_t vertex_uniforms;
		scene_fragment_uniforms_t fragment_uniforms = {
			.light_direction = {light->direction[0], light->direction[1], light->direction[2], 0.0f},
			.light_color_intensity = {light->color[0], light->color[1], light->color[2], light->intensity},
			.camera_position_time = {frame->camera->x, frame->camera->y, frame->camera->z, frame->time_seconds},
			.albedo = {renderable->color[0], renderable->color[1], renderable->color[2], 1.0f},
		};
		math3d_mat4_model(vertex_uniforms.model_matrix, transform->position, transform->rotation,
						  transform->scale);
		SDL_memcpy(vertex_uniforms.view_projection, frame->camera->matrix,
				   sizeof(vertex_uniforms.view_projection));
		SDL_memcpy(vertex_uniforms.light_matrix, renderer->shadow_camera.matrix,
				   sizeof(vertex_uniforms.light_matrix));
		SDL_PushGPUVertexUniformData(frame->commands, 0, &vertex_uniforms, sizeof(vertex_uniforms));
		SDL_PushGPUFragmentUniformData(frame->commands, 0, &fragment_uniforms, sizeof(fragment_uniforms));
		SDL_DrawGPUPrimitives(render_pass, renderer->meshes.box_vertex_count, 1, 0, 0);
	}
	SDL_EndGPURenderPass(render_pass);
}

static void scene_pass_destroy(renderer_t* renderer, render_pass_t* pass) {
	scene_pass_state_t* state = pass->state;
	if (state) {
		if (state->pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->pipeline);
		}
		SDL_free(state);
		pass->state = NULL;
	}
}

render_pass_t render_pass_scene_create(void) {
	return (render_pass_t){
		.name = "scene",
		.init = scene_pass_init,
		.resize = scene_pass_resize,
		.execute = scene_pass_execute,
		.destroy = scene_pass_destroy,
	};
}
