#pragma once

#include <SDL3/SDL_gpu.h>
#include <stdbool.h>

typedef struct {
	SDL_GPUDevice* device;
	SDL_GPUShaderFormat format;
	const char* extension;
	const char* entrypoint;
} shader_library_t;

bool shader_library_init(shader_library_t* library, SDL_GPUDevice* device);
SDL_GPUShader* shader_library_load(shader_library_t* library, const char* file,
								   SDL_GPUShaderStage stage, Uint32 uniform_buffers,
								   Uint32 samplers);
