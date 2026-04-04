#include "render_pass_shadow.h"
#include "helpers.h"
#include "math3d.h"
#include "mesh.h"
#include "renderer.h"
#include "shader.h"

typedef struct {
	SDL_GPUGraphicsPipeline* pipeline;
} shadow_pass_state_t;

typedef struct {
	float light_matrix[4][4];
	float model_matrix[4][4];
} shadow_vertex_uniforms_t;

static bool shadow_pass_init(renderer_t* renderer, render_pass_t* pass) {
	shadow_pass_state_t* state = SDL_calloc(1, sizeof(*state));
	if (!state) {
		return false;
	}
	SDL_GPUShader* vertex = shader_library_load(&renderer->shaders, "shadow.vert",
											 SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
	SDL_GPUShader* fragment = shader_library_load(&renderer->shaders, "shadow.frag",
											   SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0);
	if (!vertex || !fragment) {
		goto fail;
	}

	SDL_GPUVertexBufferDescription vertex_buffer = {
		.slot = 0,
		.pitch = sizeof(float) * 6,
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
	};
	SDL_GPUVertexAttribute position = {
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.offset = 0,
	};
	SDL_GPUGraphicsPipelineCreateInfo info = {
		.vertex_shader = vertex,
		.fragment_shader = fragment,
		.target_info = {
			.has_depth_stencil_target = true,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		},
		.vertex_input_state = {
			.vertex_buffer_descriptions = &vertex_buffer,
			.num_vertex_buffers = 1,
			.vertex_attributes = &position,
			.num_vertex_attributes = 1,
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
	if (!check_resource(state->pipeline, "create shadow pipeline")) {
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

static void shadow_pass_resize(renderer_t* renderer, render_pass_t* pass, Uint32 width, Uint32 height) {
	(void)renderer;
	(void)pass;
	(void)width;
	(void)height;
}

static void shadow_pass_execute(renderer_t* renderer, render_pass_t* pass, const frame_context_t* frame) {
	shadow_pass_state_t* state = pass->state;
	SDL_GPUDepthStencilTargetInfo depth = {
		.texture = renderer->shadow_depth_texture,
		.clear_depth = 1.0f,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
		.cycle = true,
	};
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(frame->commands, NULL, 0, &depth);
	if (!check_resource(render_pass, "begin shadow render pass")) {
		return;
	}

	SDL_BindGPUGraphicsPipeline(render_pass, state->pipeline);
	mesh_library_bind_box(&renderer->meshes, render_pass);

	for (entity_t entity = 0; entity < SCENE_MAX_ENTITIES; entity++) {
		if (!frame->scene->alive[entity] || !frame->scene->has_transform[entity] ||
			!frame->scene->has_renderable[entity]) {
			continue;
		}
		const renderable_component_t* renderable = &frame->scene->renderables[entity];
		if (!renderable->cast_shadow || renderable->mesh != MESH_BOX) {
			continue;
		}
		float model[4][4];
		shadow_vertex_uniforms_t uniforms;
		const transform_component_t* transform = &frame->scene->transforms[entity];
		math3d_mat4_model(model, transform->position, transform->rotation, transform->scale);
		SDL_memcpy(uniforms.light_matrix, renderer->shadow_camera.matrix, sizeof(uniforms.light_matrix));
		SDL_memcpy(uniforms.model_matrix, model, sizeof(uniforms.model_matrix));
		SDL_PushGPUVertexUniformData(frame->commands, 0, &uniforms, sizeof(uniforms));
		SDL_DrawGPUPrimitives(render_pass, renderer->meshes.box_vertex_count, 1, 0, 0);
	}
	SDL_EndGPURenderPass(render_pass);
}

static void shadow_pass_destroy(renderer_t* renderer, render_pass_t* pass) {
	shadow_pass_state_t* state = pass->state;
	if (state) {
		if (state->pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->pipeline);
		}
		SDL_free(state);
		pass->state = NULL;
	}
}

render_pass_t render_pass_shadow_create(void) {
	return (render_pass_t){
		.name = "shadow",
		.init = shadow_pass_init,
		.resize = shadow_pass_resize,
		.execute = shadow_pass_execute,
		.destroy = shadow_pass_destroy,
	};
}
