#include "render_pass_overlay.hpp"
#include "helpers.hpp"
#include "renderer.hpp"
#include "shader.hpp"

typedef struct {
	SDL_GPUGraphicsPipeline* pipeline;
} overlay_pass_state_t;

typedef struct {
	int viewport[4];
} overlay_fragment_uniforms_t;

static bool overlay_pass_init(renderer_t* renderer, render_pass_t* pass) {
	overlay_pass_state_t* state = static_cast<overlay_pass_state_t*>(SDL_calloc(1, sizeof(*state)));
	if (!state) {
		return false;
	}
	SDL_GPUShader* vertex = nullptr;
	SDL_GPUShader* fragment = nullptr;
	SDL_GPUColorTargetDescription color_target = {};
	SDL_GPUGraphicsPipelineCreateInfo info = {};

	color_target.format = renderer->swapchain_format;
	color_target.blend_state.enable_blend = true;
	color_target.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	color_target.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	color_target.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	color_target.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	color_target.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	color_target.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

	vertex = shader_library_load(&renderer->shaders, "fullscreen.vert", SDL_GPU_SHADERSTAGE_VERTEX, 0,
								 0);
	fragment =
		shader_library_load(&renderer->shaders, "overlay.frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);
	if (!vertex || !fragment) {
		goto fail;
	}

	info.vertex_shader = vertex;
	info.fragment_shader = fragment;
	info.target_info.num_color_targets = 1;
	info.target_info.color_target_descriptions = &color_target;
	state->pipeline = SDL_CreateGPUGraphicsPipeline(renderer->device, &info);
	if (!check_resource(state->pipeline, "create overlay pipeline")) {
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

static void overlay_pass_resize(renderer_t* renderer, render_pass_t* pass, Uint32 width, Uint32 height) {
	(void)renderer;
	(void)pass;
	(void)width;
	(void)height;
}

static void overlay_pass_execute(renderer_t* renderer, render_pass_t* pass, const frame_context_t* frame) {
	overlay_pass_state_t* state = static_cast<overlay_pass_state_t*>(pass->state);
	overlay_fragment_uniforms_t uniforms = {
		.viewport = {0, 0, (int)renderer->window_width, (int)renderer->window_height},
	};
	SDL_GPUColorTargetInfo color = {
		.texture = frame->swapchain_texture,
		.load_op = SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
	};
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(frame->commands, &color, 1, NULL);
	if (!check_resource(render_pass, "begin overlay render pass")) {
		return;
	}

	SDL_BindGPUGraphicsPipeline(render_pass, state->pipeline);
	SDL_PushGPUFragmentUniformData(frame->commands, 0, &uniforms, sizeof(uniforms));
	SDL_DrawGPUPrimitives(render_pass, 4, 1, 0, 0);
	SDL_EndGPURenderPass(render_pass);
}

static void overlay_pass_destroy(renderer_t* renderer, render_pass_t* pass) {
	overlay_pass_state_t* state = static_cast<overlay_pass_state_t*>(pass->state);
	if (state) {
		if (state->pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->pipeline);
		}
		SDL_free(state);
		pass->state = NULL;
	}
}

render_pass_t render_pass_overlay_create(void) {
	return (render_pass_t){
		.name = "overlay",
		.init = overlay_pass_init,
		.resize = overlay_pass_resize,
		.execute = overlay_pass_execute,
		.destroy = overlay_pass_destroy,
	};
}
