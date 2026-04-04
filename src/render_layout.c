#include "render_layout.h"
#include <SDL3/SDL_gpu.h>
#include <assert.h>
#include <string.h>

static const render_backend_layout_t RENDER_LAYOUT_DEFAULT = {
	.shadow_vertex_uniform_slots = {0, 1},
	.composite_sampler_order = {
		COMPOSITE_SAMPLER_ATLAS,
		COMPOSITE_SAMPLER_POSITION,
		COMPOSITE_SAMPLER_UV,
		COMPOSITE_SAMPLER_VOXEL,
		COMPOSITE_SAMPLER_SHADOW,
		COMPOSITE_SAMPLER_SSAO,
	},
	.composite_fragment_uniform_slots = {0, 1, 2},
	.transparent_vertex_uniform_slots = {1, 2, 3},
	.transparent_fragment_uniform_slots = {0, 1},
};

static const render_backend_layout_t RENDER_LAYOUT_METAL = {
	.shadow_vertex_uniform_slots = {1, 0},
	.composite_sampler_order = {
		COMPOSITE_SAMPLER_POSITION,
		COMPOSITE_SAMPLER_VOXEL,
		COMPOSITE_SAMPLER_UV,
		COMPOSITE_SAMPLER_ATLAS,
		COMPOSITE_SAMPLER_SHADOW,
		COMPOSITE_SAMPLER_SSAO,
	},
	.composite_fragment_uniform_slots = {1, 2, 0},
	.transparent_vertex_uniform_slots = {2, 1, 3},
	.transparent_fragment_uniform_slots = {1, 0},
};

const render_backend_layout_t* render_layout_get(bool gpu_is_metal) {
	return gpu_is_metal ? &RENDER_LAYOUT_METAL : &RENDER_LAYOUT_DEFAULT;
}

bool render_layout_is_metal(SDL_GPUDevice* device) {
	const char* driver = SDL_GetGPUDeviceDriver(device);
	return driver && strcmp(driver, "metal") == 0;
}

void render_layout_bind_fragment_samplers(SDL_GPURenderPass* pass,
										  const SDL_GPUTextureSamplerBinding* bindings,
										  const Uint32* order, const Uint32 count) {
	SDL_GPUTextureSamplerBinding mapped[COMPOSITE_SAMPLER_COUNT] = {0};
	assert(count <= SDL_arraysize(mapped));
	for (Uint32 i = 0; i < count; i++) {
		mapped[i] = bindings[order[i]];
	}
	SDL_BindGPUFragmentSamplers(pass, 0, mapped, count);
}
