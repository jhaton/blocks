#pragma once

#include <SDL3/SDL_gpu.h>
#include <stdbool.h>

typedef struct {
	SDL_GPUDevice* device;
	SDL_GPUBuffer* box_vertex_buffer;
	Uint32 box_vertex_count;
} mesh_library_t;

bool mesh_library_init(mesh_library_t* library, SDL_GPUDevice* device);
void mesh_library_destroy(mesh_library_t* library);
void mesh_library_bind_box(const mesh_library_t* library, SDL_GPURenderPass* pass);
