#pragma once

#include "camera.hpp"
#include "mesh.hpp"
#include "render_pass.hpp"
#include "shader.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdbool.h>

typedef struct renderer {
	SDL_Window* window;
	SDL_GPUDevice* device;
	SDL_GPUTextureFormat swapchain_format;
	SDL_GPUTextureFormat scene_color_format;
	Uint32 window_width;
	Uint32 window_height;

	shader_library_t shaders;
	mesh_library_t meshes;
	camera_t shadow_camera;

	SDL_GPUTexture* shadow_depth_texture;
	SDL_GPUTexture* scene_color_texture;
	SDL_GPUTexture* scene_depth_texture;
	SDL_GPUSampler* linear_sampler;
	SDL_GPUSampler* shadow_sampler;

	render_pass_t passes[3];
	Uint32 pass_count;
} renderer_t;

bool renderer_init(renderer_t* renderer, SDL_Window* window, bool validation);
void renderer_destroy(renderer_t* renderer);
void renderer_draw(renderer_t* renderer, const scene_t* scene, const camera_t* camera, float time_seconds);
