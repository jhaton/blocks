#include "shader.h"
#include "helpers.h"
#include <string.h>

bool shader_library_init(shader_library_t* library, SDL_GPUDevice* device) {
	assert(library);
	assert(device);
	library->device = device;
	const SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
	if (formats == SDL_GPU_SHADERFORMAT_INVALID) {
		SDL_Log("Failed to query shader formats: %s", SDL_GetError());
		return false;
	}
	if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
		library->format = SDL_GPU_SHADERFORMAT_SPIRV;
		library->extension = ".spv";
		library->entrypoint = "main";
		return true;
	}
	if (formats & SDL_GPU_SHADERFORMAT_MSL) {
		library->format = SDL_GPU_SHADERFORMAT_MSL;
		library->extension = ".msl";
		library->entrypoint = "main0";
		return true;
	}
	SDL_Log("Unsupported shader format flags: 0x%x", formats);
	return false;
}

SDL_GPUShader* shader_library_load(shader_library_t* library, const char* file,
								   SDL_GPUShaderStage stage, Uint32 uniform_buffers,
								   Uint32 samplers) {
	assert(library);
	assert(file);
	char path[128];
	SDL_snprintf(path, sizeof(path), "%s%s", file, library->extension);
	SDL_GPUShaderCreateInfo info = {0};
	void* code = SDL_LoadFile(path, &info.code_size);
	if (!code) {
		SDL_Log("Failed to load %s: %s", path, SDL_GetError());
		return NULL;
	}
	info.code = code;
	info.entrypoint = library->entrypoint;
	info.format = library->format;
	info.stage = stage;
	info.num_uniform_buffers = uniform_buffers;
	info.num_samplers = samplers;
	SDL_GPUShader* shader = SDL_CreateGPUShader(library->device, &info);
	SDL_free(code);
	if (!shader) {
		SDL_Log("Failed to create shader %s: %s", file, SDL_GetError());
	}
	return shader;
}
