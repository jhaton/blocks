#include "mesh.hpp"
#include "helpers.hpp"

typedef struct {
	float position[3];
	float normal[3];
} mesh_vertex_t;

bool mesh_library_init(mesh_library_t* library, SDL_GPUDevice* device) {
	assert(library);
	assert(device);
	static const mesh_vertex_t BOX_VERTICES[] = {
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}}, {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},  {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

		{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},  {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},   {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},

		{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},  {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},

		{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},

		{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}}, {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},  {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
	};

	library->device = device;
	library->box_vertex_count = SDL_arraysize(BOX_VERTICES);

	SDL_GPUBufferCreateInfo buffer_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = sizeof(BOX_VERTICES),
	};
	library->box_vertex_buffer = SDL_CreateGPUBuffer(device, &buffer_info);
	if (!check_resource(library->box_vertex_buffer, "create mesh vertex buffer")) {
		return false;
	}

	SDL_GPUTransferBufferCreateInfo transfer_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = sizeof(BOX_VERTICES),
	};
	SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
	if (!check_resource(transfer, "create mesh transfer buffer")) {
		return false;
	}

	void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
	if (!check_resource(mapped, "map mesh transfer buffer")) {
		SDL_ReleaseGPUTransferBuffer(device, transfer);
		return false;
	}
	SDL_memcpy(mapped, BOX_VERTICES, sizeof(BOX_VERTICES));
	SDL_UnmapGPUTransferBuffer(device, transfer);

	SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(device);
	if (!check_resource(commands, "acquire mesh upload command buffer")) {
		SDL_ReleaseGPUTransferBuffer(device, transfer);
		return false;
	}
	SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(commands);
	if (!check_resource(pass, "begin mesh copy pass")) {
		SDL_CancelGPUCommandBuffer(commands);
		SDL_ReleaseGPUTransferBuffer(device, transfer);
		return false;
	}
	SDL_GPUTransferBufferLocation location = {.transfer_buffer = transfer};
	SDL_GPUBufferRegion region = {.buffer = library->box_vertex_buffer, .size = sizeof(BOX_VERTICES)};
	SDL_UploadToGPUBuffer(pass, &location, &region, false);
	SDL_EndGPUCopyPass(pass);
	SDL_SubmitGPUCommandBuffer(commands);
	SDL_ReleaseGPUTransferBuffer(device, transfer);
	return true;
}

void mesh_library_destroy(mesh_library_t* library) {
	assert(library);
	if (library->box_vertex_buffer) {
		SDL_ReleaseGPUBuffer(library->device, library->box_vertex_buffer);
		library->box_vertex_buffer = NULL;
	}
	library->device = NULL;
}

void mesh_library_bind_box(const mesh_library_t* library, SDL_GPURenderPass* pass) {
	assert(library);
	assert(pass);
	SDL_GPUBufferBinding binding = {.buffer = library->box_vertex_buffer};
	SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
}
