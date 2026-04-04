#pragma once

#include "camera.h"
#include "scene.h"
#include <SDL3/SDL_gpu.h>
#include <stdbool.h>

struct renderer;

typedef struct {
	SDL_GPUCommandBuffer* commands;
	SDL_GPUTexture* swapchain_texture;
	const scene_t* scene;
	const camera_t* camera;
	float time_seconds;
} frame_context_t;

typedef struct render_pass {
	const char* name;
	bool (*init)(struct renderer* renderer, struct render_pass* pass);
	void (*resize)(struct renderer* renderer, struct render_pass* pass, Uint32 width, Uint32 height);
	void (*execute)(struct renderer* renderer, struct render_pass* pass, const frame_context_t* frame);
	void (*destroy)(struct renderer* renderer, struct render_pass* pass);
	void* state;
} render_pass_t;
