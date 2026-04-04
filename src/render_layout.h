#pragma once

#include <SDL3/SDL.h>

typedef enum {
	COMPOSITE_SAMPLER_ATLAS,
	COMPOSITE_SAMPLER_POSITION,
	COMPOSITE_SAMPLER_UV,
	COMPOSITE_SAMPLER_VOXEL,
	COMPOSITE_SAMPLER_SHADOW,
	COMPOSITE_SAMPLER_SSAO,
	COMPOSITE_SAMPLER_COUNT
} composite_sampler_t;

typedef enum {
	COMPOSITE_UNIFORM_PLAYER_POSITION,
	COMPOSITE_UNIFORM_SHADOW_VECTOR,
	COMPOSITE_UNIFORM_SHADOW_MATRIX,
	COMPOSITE_UNIFORM_COUNT
} composite_uniform_t;

typedef enum {
	TRANSPARENT_VERTEX_UNIFORM_PLAYER_MATRIX,
	TRANSPARENT_VERTEX_UNIFORM_PLAYER_POSITION,
	TRANSPARENT_VERTEX_UNIFORM_SHADOW_MATRIX,
	TRANSPARENT_VERTEX_UNIFORM_COUNT
} transparent_vertex_uniform_t;

typedef enum {
	TRANSPARENT_FRAGMENT_UNIFORM_SHADOW_VECTOR,
	TRANSPARENT_FRAGMENT_UNIFORM_PLAYER_POSITION,
	TRANSPARENT_FRAGMENT_UNIFORM_COUNT
} transparent_fragment_uniform_t;

typedef struct {
	Uint32 shadow_vertex_uniform_slots[2];
	Uint32 composite_sampler_order[COMPOSITE_SAMPLER_COUNT];
	Uint32 composite_fragment_uniform_slots[COMPOSITE_UNIFORM_COUNT];
	Uint32 transparent_vertex_uniform_slots[TRANSPARENT_VERTEX_UNIFORM_COUNT];
	Uint32 transparent_fragment_uniform_slots[TRANSPARENT_FRAGMENT_UNIFORM_COUNT];
} render_backend_layout_t;

const render_backend_layout_t* render_layout_get(bool gpu_is_metal);
bool render_layout_is_metal(SDL_GPUDevice* device);
void render_layout_bind_fragment_samplers(SDL_GPURenderPass* pass,
										  const SDL_GPUTextureSamplerBinding* bindings,
										  const Uint32* order, Uint32 count);
