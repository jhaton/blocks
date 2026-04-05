#include "renderer.h"
#include "config.h"
#include "helpers.h"
#include "math3d.h"
#include "render_pass_overlay.h"
#include "render_pass_scene.h"
#include "render_pass_shadow.h"
#include <math.h>

static bool create_samplers(renderer_t* renderer) {
	SDL_GPUSamplerCreateInfo linear_info = {
		.min_filter = SDL_GPU_FILTER_LINEAR,
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	};
	renderer->linear_sampler = SDL_CreateGPUSampler(renderer->device, &linear_info);
	if (!check_resource(renderer->linear_sampler, "create linear sampler")) {
		return false;
	}

	SDL_GPUSamplerCreateInfo shadow_info = linear_info;
	shadow_info.min_filter = SDL_GPU_FILTER_NEAREST;
	shadow_info.mag_filter = SDL_GPU_FILTER_NEAREST;
	renderer->shadow_sampler = SDL_CreateGPUSampler(renderer->device, &shadow_info);
	return check_resource(renderer->shadow_sampler, "create shadow sampler");
}

static void destroy_textures(renderer_t* renderer) {
	if (renderer->scene_color_texture) {
		SDL_ReleaseGPUTexture(renderer->device, renderer->scene_color_texture);
		renderer->scene_color_texture = NULL;
	}
	if (renderer->scene_depth_texture) {
		SDL_ReleaseGPUTexture(renderer->device, renderer->scene_depth_texture);
		renderer->scene_depth_texture = NULL;
	}
	if (renderer->shadow_depth_texture) {
		SDL_ReleaseGPUTexture(renderer->device, renderer->shadow_depth_texture);
		renderer->shadow_depth_texture = NULL;
	}
}

static bool create_textures(renderer_t* renderer, Uint32 width, Uint32 height) {
	destroy_textures(renderer);

	SDL_GPUTextureCreateInfo shadow_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.width = SHADOW_MAP_SIZE,
		.height = SHADOW_MAP_SIZE,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
	};
	renderer->shadow_depth_texture = SDL_CreateGPUTexture(renderer->device, &shadow_info);
	if (!check_resource(renderer->shadow_depth_texture, "create shadow depth texture")) {
		return false;
	}

	SDL_GPUTextureCreateInfo color_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = renderer->scene_color_format,
		.width = width,
		.height = height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
	};
	renderer->scene_color_texture = SDL_CreateGPUTexture(renderer->device, &color_info);
	if (!check_resource(renderer->scene_color_texture, "create scene color texture")) {
		return false;
	}

	SDL_GPUTextureCreateInfo depth_info = color_info;
	depth_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	depth_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	renderer->scene_depth_texture = SDL_CreateGPUTexture(renderer->device, &depth_info);
	return check_resource(renderer->scene_depth_texture, "create scene depth texture");
}

static bool resize_targets(renderer_t* renderer, Uint32 width, Uint32 height) {
	if (width == renderer->window_width && height == renderer->window_height &&
		renderer->scene_color_texture && renderer->scene_depth_texture) {
		return true;
	}
	renderer->window_width = width;
	renderer->window_height = height;
	for (Uint32 i = 0; i < renderer->pass_count; i++) {
		renderer->passes[i].resize(renderer, &renderer->passes[i], width, height);
	}
	return create_textures(renderer, width, height);
}

static void update_shadow_camera(renderer_t* renderer, const scene_t* scene, const camera_t* camera) {
	const directional_light_component_t* light = scene_main_light(scene);
	assert(light);
	float focus[3] = {camera->x, 0.0f, camera->z};
	float direction[3] = {light->direction[0], light->direction[1], light->direction[2]};
	math3d_vec3_normalize(direction);

	const float pitch = asinf(direction[1]);
	const float yaw = atan2f(direction[0], -direction[2]);
	camera_set_rotation(&renderer->shadow_camera, pitch, yaw);
	camera_set_position(&renderer->shadow_camera,
						focus[0] - direction[0] * SHADOW_DISTANCE,
						SHADOW_CAMERA_HEIGHT - direction[1] * SHADOW_DISTANCE,
						focus[2] - direction[2] * SHADOW_DISTANCE);
	camera_update(&renderer->shadow_camera);
}

bool renderer_init(renderer_t* renderer, SDL_Window* window, bool validation) {
	assert(renderer);
	assert(window);
	SDL_zero(*renderer);
	renderer->window = window;
	renderer->device =
		SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, validation, NULL);
	if (!check_resource(renderer->device, "create gpu device")) {
		return false;
	}
	if (!SDL_ClaimWindowForGPUDevice(renderer->device, window)) {
		SDL_Log("Failed to claim window for GPU device: %s", SDL_GetError());
		return false;
	}
	renderer->swapchain_format = SDL_GetGPUSwapchainTextureFormat(renderer->device, window);
	renderer->scene_color_format = renderer->swapchain_format;

	camera_init(&renderer->shadow_camera, CAMERA_TYPE_ORTHO);
	renderer->shadow_camera.ortho = SHADOW_DISTANCE;
	renderer->shadow_camera.near = -SHADOW_DISTANCE;
	renderer->shadow_camera.far = SHADOW_DISTANCE * 2.0f;

	if (!shader_library_init(&renderer->shaders, renderer->device)) {
		return false;
	}
	if (!mesh_library_init(&renderer->meshes, renderer->device)) {
		return false;
	}
	if (!create_samplers(renderer)) {
		return false;
	}

	renderer->passes[0] = render_pass_shadow_create();
	renderer->passes[1] = render_pass_scene_create();
	renderer->passes[2] = render_pass_overlay_create();
	renderer->pass_count = 3;
	for (Uint32 i = 0; i < renderer->pass_count; i++) {
		if (!renderer->passes[i].init(renderer, &renderer->passes[i])) {
			SDL_Log("Failed to initialize render pass: %s", renderer->passes[i].name);
			return false;
		}
	}

	int width = 0;
	int height = 0;
	SDL_GetWindowSizeInPixels(window, &width, &height);
	return resize_targets(renderer, (Uint32)width, (Uint32)height);
}

void renderer_destroy(renderer_t* renderer) {
	assert(renderer);
	for (Uint32 i = 0; i < renderer->pass_count; i++) {
		renderer->passes[i].destroy(renderer, &renderer->passes[i]);
	}
	destroy_textures(renderer);
	if (renderer->shadow_sampler) {
		SDL_ReleaseGPUSampler(renderer->device, renderer->shadow_sampler);
	}
	if (renderer->linear_sampler) {
		SDL_ReleaseGPUSampler(renderer->device, renderer->linear_sampler);
	}
	mesh_library_destroy(&renderer->meshes);
	if (renderer->device) {
		SDL_ReleaseWindowFromGPUDevice(renderer->device, renderer->window);
		SDL_DestroyGPUDevice(renderer->device);
	}
	SDL_zero(*renderer);
}

void renderer_draw(renderer_t* renderer, const scene_t* scene, const camera_t* camera, float time_seconds) {
	assert(renderer);
	assert(scene);
	assert(camera);

	SDL_WaitForGPUSwapchain(renderer->device, renderer->window);
	SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(renderer->device);
	if (!check_resource(commands, "acquire frame command buffer")) {
		return;
	}

	SDL_GPUTexture* swapchain_texture = NULL;
	Uint32 width = 0;
	Uint32 height = 0;
	if (!SDL_AcquireGPUSwapchainTexture(commands, renderer->window, &swapchain_texture, &width, &height)) {
		SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
		SDL_CancelGPUCommandBuffer(commands);
		return;
	}
	if (!swapchain_texture || !width || !height) {
		SDL_SubmitGPUCommandBuffer(commands);
		return;
	}
	if (!resize_targets(renderer, width, height)) {
		SDL_Log("Failed to resize render targets");
		SDL_CancelGPUCommandBuffer(commands);
		return;
	}

	update_shadow_camera(renderer, scene, camera);

	frame_context_t frame = {
		.commands = commands,
		.swapchain_texture = swapchain_texture,
		.scene = scene,
		.camera = camera,
		.time_seconds = time_seconds,
	};
	for (Uint32 i = 0; i + 1 < renderer->pass_count; i++) {
		SDL_PushGPUDebugGroup(commands, renderer->passes[i].name);
		renderer->passes[i].execute(renderer, &renderer->passes[i], &frame);
		SDL_PopGPUDebugGroup(commands);
	}

	SDL_GPUBlitInfo blit = {
		.source = {
			.texture = renderer->scene_color_texture,
			.w = renderer->window_width,
			.h = renderer->window_height,
		},
		.destination = {
			.texture = swapchain_texture,
			.w = renderer->window_width,
			.h = renderer->window_height,
		},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.filter = SDL_GPU_FILTER_LINEAR,
	};
	SDL_BlitGPUTexture(commands, &blit);

	SDL_PushGPUDebugGroup(commands, renderer->passes[renderer->pass_count - 1].name);
	renderer->passes[2].execute(renderer, &renderer->passes[2], &frame);
	SDL_PopGPUDebugGroup(commands);
	SDL_SubmitGPUCommandBuffer(commands);
}
