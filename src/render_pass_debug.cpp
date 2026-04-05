#include "render_pass_debug.hpp"

#include "helpers.hpp"
#include "renderer.hpp"
#include "shader.hpp"

namespace {

constexpr Uint32 kMaxDebugVertices = 8192;

struct debug_vertex_t {
	float position[3];
	float color[4];
};

struct debug_pass_state_t {
	SDL_GPUGraphicsPipeline* pipeline = nullptr;
	SDL_GPUBuffer* vertex_buffer = nullptr;
	SDL_GPUTransferBuffer* transfer_buffer = nullptr;
};

struct debug_vertex_uniforms_t {
	float view_projection[4][4];
};

struct line_builder_t {
	debug_vertex_t* vertices = nullptr;
	Uint32 count = 0;
};

void add_line(line_builder_t* builder, const float a[3], const float b[3], const float color[4]) {
	if (builder->count + 2 > kMaxDebugVertices) {
		return;
	}
	SDL_memcpy(builder->vertices[builder->count].position, a, sizeof(builder->vertices[builder->count].position));
	SDL_memcpy(builder->vertices[builder->count].color, color, sizeof(builder->vertices[builder->count].color));
	builder->count++;
	SDL_memcpy(builder->vertices[builder->count].position, b, sizeof(builder->vertices[builder->count].position));
	SDL_memcpy(builder->vertices[builder->count].color, color, sizeof(builder->vertices[builder->count].color));
	builder->count++;
}

void add_box(line_builder_t* builder, const float center[3], const float half_extents[3], const float color[4]) {
	const float x0 = center[0] - half_extents[0];
	const float x1 = center[0] + half_extents[0];
	const float y0 = center[1] - half_extents[1];
	const float y1 = center[1] + half_extents[1];
	const float z0 = center[2] - half_extents[2];
	const float z1 = center[2] + half_extents[2];
	const float c000[3] = {x0, y0, z0};
	const float c001[3] = {x0, y0, z1};
	const float c010[3] = {x0, y1, z0};
	const float c011[3] = {x0, y1, z1};
	const float c100[3] = {x1, y0, z0};
	const float c101[3] = {x1, y0, z1};
	const float c110[3] = {x1, y1, z0};
	const float c111[3] = {x1, y1, z1};

	add_line(builder, c000, c001, color);
	add_line(builder, c000, c010, color);
	add_line(builder, c000, c100, color);
	add_line(builder, c001, c011, color);
	add_line(builder, c001, c101, color);
	add_line(builder, c010, c011, color);
	add_line(builder, c010, c110, color);
	add_line(builder, c100, c101, color);
	add_line(builder, c100, c110, color);
	add_line(builder, c111, c011, color);
	add_line(builder, c111, c101, color);
	add_line(builder, c111, c110, color);
}

void add_axes(line_builder_t* builder, const float origin[3], const float length) {
	const float red[4] = {1.0f, 0.25f, 0.25f, 0.95f};
	const float green[4] = {0.30f, 0.90f, 0.45f, 0.95f};
	const float blue[4] = {0.35f, 0.65f, 1.0f, 0.95f};
	float x[3] = {origin[0] + length, origin[1], origin[2]};
	float y[3] = {origin[0], origin[1] + length, origin[2]};
	float z[3] = {origin[0], origin[1], origin[2] + length};
	add_line(builder, origin, x, red);
	add_line(builder, origin, y, green);
	add_line(builder, origin, z, blue);
}

void normalize(float v[3]) {
	float length = SDL_sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (length <= EPSILON) {
		return;
	}
	v[0] /= length;
	v[1] /= length;
	v[2] /= length;
}

void cross(float out[3], const float a[3], const float b[3]) {
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

void build_shadow_debug(line_builder_t* builder, const camera_t* shadow_camera) {
	const float yellow[4] = {0.98f, 0.86f, 0.28f, 0.95f};
	const float orange[4] = {1.0f, 0.62f, 0.18f, 0.95f};
	float forward[3];
	camera_get_vector(shadow_camera, &forward[0], &forward[1], &forward[2]);
	normalize(forward);
	float up[3] = {0.0f, 1.0f, 0.0f};
	float right[3];
	cross(right, forward, up);
	normalize(right);
	cross(up, right, forward);
	normalize(up);

	const float origin[3] = {shadow_camera->x, shadow_camera->y, shadow_camera->z};
	float tip[3] = {origin[0] + forward[0] * 8.0f, origin[1] + forward[1] * 8.0f,
					origin[2] + forward[2] * 8.0f};
	add_line(builder, origin, tip, yellow);
	add_axes(builder, origin, 1.5f);

	float square_center[3] = {tip[0] + forward[0] * 3.0f, tip[1] + forward[1] * 3.0f,
							  tip[2] + forward[2] * 3.0f};
	float r[3] = {right[0] * shadow_camera->ortho * 0.35f, right[1] * shadow_camera->ortho * 0.35f,
				  right[2] * shadow_camera->ortho * 0.35f};
	float u[3] = {up[0] * shadow_camera->ortho * 0.35f, up[1] * shadow_camera->ortho * 0.35f,
				  up[2] * shadow_camera->ortho * 0.35f};
	float a[3] = {square_center[0] - r[0] - u[0], square_center[1] - r[1] - u[1],
				  square_center[2] - r[2] - u[2]};
	float b[3] = {square_center[0] + r[0] - u[0], square_center[1] + r[1] - u[1],
				  square_center[2] + r[2] - u[2]};
	float c[3] = {square_center[0] + r[0] + u[0], square_center[1] + r[1] + u[1],
				  square_center[2] + r[2] + u[2]};
	float d[3] = {square_center[0] - r[0] + u[0], square_center[1] - r[1] + u[1],
				  square_center[2] - r[2] + u[2]};
	add_line(builder, a, b, orange);
	add_line(builder, b, c, orange);
	add_line(builder, c, d, orange);
	add_line(builder, d, a, orange);
}

bool debug_pass_init(renderer_t* renderer, render_pass_t* pass) {
	debug_pass_state_t* state = static_cast<debug_pass_state_t*>(SDL_calloc(1, sizeof(*state)));
	if (!state) {
		return false;
	}

	SDL_GPUShader* vertex = nullptr;
	SDL_GPUShader* fragment = nullptr;
	SDL_GPUVertexBufferDescription vertex_buffer = {};
	SDL_GPUVertexAttribute attributes[2] = {};
	SDL_GPUColorTargetDescription color_target = {};
	SDL_GPUGraphicsPipelineCreateInfo info = {};
	SDL_GPUBufferCreateInfo buffer_info = {};
	SDL_GPUTransferBufferCreateInfo transfer_info = {};

	vertex = shader_library_load(&renderer->shaders, "debug_lines.vert", SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
	fragment =
		shader_library_load(&renderer->shaders, "debug_lines.frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0);
	if (!vertex || !fragment) {
		goto fail;
	}

	vertex_buffer.slot = 0;
	vertex_buffer.pitch = sizeof(debug_vertex_t);
	vertex_buffer.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	attributes[0].location = 0;
	attributes[0].buffer_slot = 0;
	attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	attributes[0].offset = 0;
	attributes[1].location = 1;
	attributes[1].buffer_slot = 0;
	attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	attributes[1].offset = sizeof(float) * 3;

	color_target.format = renderer->swapchain_format;
	color_target.blend_state.enable_blend = true;
	color_target.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	color_target.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	color_target.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	color_target.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	color_target.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	color_target.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

	info.vertex_shader = vertex;
	info.fragment_shader = fragment;
	info.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
	info.target_info.num_color_targets = 1;
	info.target_info.color_target_descriptions = &color_target;
	info.vertex_input_state.vertex_buffer_descriptions = &vertex_buffer;
	info.vertex_input_state.num_vertex_buffers = 1;
	info.vertex_input_state.vertex_attributes = attributes;
	info.vertex_input_state.num_vertex_attributes = SDL_arraysize(attributes);

	state->pipeline = SDL_CreateGPUGraphicsPipeline(renderer->device, &info);
	if (!check_resource(state->pipeline, "create debug pipeline")) {
		goto fail;
	}

	buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	buffer_info.size = sizeof(debug_vertex_t) * kMaxDebugVertices;
	state->vertex_buffer = SDL_CreateGPUBuffer(renderer->device, &buffer_info);
	if (!check_resource(state->vertex_buffer, "create debug vertex buffer")) {
		goto fail;
	}

	transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_info.size = sizeof(debug_vertex_t) * kMaxDebugVertices;
	state->transfer_buffer = SDL_CreateGPUTransferBuffer(renderer->device, &transfer_info);
	if (!check_resource(state->transfer_buffer, "create debug transfer buffer")) {
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
	if (state) {
		if (state->vertex_buffer) {
			SDL_ReleaseGPUBuffer(renderer->device, state->vertex_buffer);
		}
		if (state->transfer_buffer) {
			SDL_ReleaseGPUTransferBuffer(renderer->device, state->transfer_buffer);
		}
		SDL_free(state);
	}
	return false;
}

void debug_pass_resize(renderer_t* renderer, render_pass_t* pass, Uint32 width, Uint32 height) {
	(void)renderer;
	(void)pass;
	(void)width;
	(void)height;
}

void debug_pass_execute(renderer_t* renderer, render_pass_t* pass, const frame_context_t* frame) {
	debug_pass_state_t* state = static_cast<debug_pass_state_t*>(pass->state);
	if (!frame->debug || !frame->debug->enabled) {
		return;
	}
	if (!frame->debug->draw_colliders && !frame->debug->draw_player_body && !frame->debug->draw_axes &&
		!frame->debug->draw_shadow) {
		return;
	}

	void* mapped = SDL_MapGPUTransferBuffer(renderer->device, state->transfer_buffer, false);
	if (!check_resource(mapped, "map debug transfer buffer")) {
		return;
	}
	line_builder_t builder = {
		.vertices = static_cast<debug_vertex_t*>(mapped),
		.count = 0,
	};

	const float collider_color[4] = {0.98f, 0.52f, 0.28f, 0.85f};
	const float player_color[4] = {0.38f, 0.92f, 0.94f, 0.95f};
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; ++entity) {
		if (!frame->scene->alive[entity] || !frame->scene->has_transform[entity]) {
			continue;
		}
		const transform_component_t& transform = frame->scene->transforms[entity];
		if (frame->debug->draw_colliders && frame->scene->has_box_collider[entity] &&
			frame->scene->box_colliders[entity].solid) {
			add_box(&builder, transform.position, frame->scene->box_colliders[entity].half_extents,
					collider_color);
		}
		if (frame->debug->draw_axes) {
			add_axes(&builder, transform.position, 1.2f);
		}
	}

	const entity_t player = scene_player(frame->scene);
	if (frame->debug->draw_player_body && player < starter::config::kSceneMaxEntities &&
		frame->scene->alive[player] && frame->scene->has_transform[player] &&
		frame->scene->has_character_body[player]) {
		const transform_component_t& transform = frame->scene->transforms[player];
		const character_body_component_t& body = frame->scene->character_bodies[player];
		const float body_center[3] = {transform.position[0],
									  transform.position[1] - body.eye_offset + body.height * 0.5f,
									  transform.position[2]};
		const float body_extents[3] = {body.radius, body.height * 0.5f, body.radius};
		add_box(&builder, body_center, body_extents, player_color);
	}

	if (frame->debug->draw_shadow) {
		build_shadow_debug(&builder, &renderer->shadow_camera);
	}

	SDL_UnmapGPUTransferBuffer(renderer->device, state->transfer_buffer);
	if (builder.count == 0) {
		return;
	}

	SDL_GPUCommandBuffer* commands = frame->commands;
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(commands);
	if (!check_resource(copy_pass, "begin debug copy pass")) {
		return;
	}
	SDL_GPUTransferBufferLocation location = {.transfer_buffer = state->transfer_buffer};
	SDL_GPUBufferRegion region = {
		.buffer = state->vertex_buffer,
		.size = static_cast<Uint32>(sizeof(debug_vertex_t) * builder.count),
	};
	SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
	SDL_EndGPUCopyPass(copy_pass);

	SDL_GPUColorTargetInfo color = {
		.texture = frame->swapchain_texture,
		.load_op = SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
	};
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(commands, &color, 1, nullptr);
	if (!check_resource(render_pass, "begin debug render pass")) {
		return;
	}

	debug_vertex_uniforms_t uniforms;
	SDL_memcpy(uniforms.view_projection, frame->camera->matrix, sizeof(uniforms.view_projection));
	SDL_GPUBufferBinding binding = {.buffer = state->vertex_buffer};
	SDL_BindGPUGraphicsPipeline(render_pass, state->pipeline);
	SDL_BindGPUVertexBuffers(render_pass, 0, &binding, 1);
	SDL_PushGPUVertexUniformData(commands, 0, &uniforms, sizeof(uniforms));
	SDL_DrawGPUPrimitives(render_pass, builder.count, 1, 0, 0);
	SDL_EndGPURenderPass(render_pass);
}

void debug_pass_destroy(renderer_t* renderer, render_pass_t* pass) {
	debug_pass_state_t* state = static_cast<debug_pass_state_t*>(pass->state);
	if (!state) {
		return;
	}
	if (state->pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->pipeline);
	}
	if (state->vertex_buffer) {
		SDL_ReleaseGPUBuffer(renderer->device, state->vertex_buffer);
	}
	if (state->transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(renderer->device, state->transfer_buffer);
	}
	SDL_free(state);
	pass->state = nullptr;
}

} // namespace

render_pass_t render_pass_debug_create(void) {
	return (render_pass_t){
		.name = "debug",
		.init = debug_pass_init,
		.resize = debug_pass_resize,
		.execute = debug_pass_execute,
		.destroy = debug_pass_destroy,
	};
}
