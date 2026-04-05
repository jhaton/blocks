#include "render_pass_overlay.hpp"

#include "helpers.hpp"
#include "renderer.hpp"
#include "shader.hpp"

namespace {

constexpr int kFontCellWidth = 8;
constexpr int kFontCellHeight = 8;
constexpr int kFontColumns = 16;
constexpr int kFontRows = 4;
constexpr int kFontTextureWidth = kFontColumns * kFontCellWidth;
constexpr int kFontTextureHeight = kFontRows * kFontCellHeight;
constexpr Uint32 kMaxTextVertices = starter::kDebugLineCount * starter::kDebugLineLength * 6;

struct overlay_text_vertex_t {
	float position[2];
	float uv[2];
	float color[4];
};

struct overlay_pass_state_t {
	SDL_GPUGraphicsPipeline* panel_pipeline = nullptr;
	SDL_GPUGraphicsPipeline* text_pipeline = nullptr;
	SDL_GPUTexture* font_texture = nullptr;
	SDL_GPUSampler* font_sampler = nullptr;
	SDL_GPUBuffer* text_vertex_buffer = nullptr;
	SDL_GPUTransferBuffer* text_transfer_buffer = nullptr;
};

struct overlay_uniforms_t {
	int viewport[4];
	int debug_state[4];
};

struct overlay_text_uniforms_t {
	float viewport[4];
};

const Uint8* glyph_rows(char c) {
	switch (c) {
	case 'A': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
		return rows;
	}
	case 'B': {
		static const Uint8 rows[] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
		return rows;
	}
	case 'C': {
		static const Uint8 rows[] = {0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F};
		return rows;
	}
	case 'D': {
		static const Uint8 rows[] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
		return rows;
	}
	case 'E': {
		static const Uint8 rows[] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
		return rows;
	}
	case 'F': {
		static const Uint8 rows[] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
		return rows;
	}
	case 'G': {
		static const Uint8 rows[] = {0x0F, 0x10, 0x10, 0x13, 0x11, 0x11, 0x0F};
		return rows;
	}
	case 'H': {
		static const Uint8 rows[] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
		return rows;
	}
	case 'I': {
		static const Uint8 rows[] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F};
		return rows;
	}
	case 'J': {
		static const Uint8 rows[] = {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E};
		return rows;
	}
	case 'K': {
		static const Uint8 rows[] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
		return rows;
	}
	case 'L': {
		static const Uint8 rows[] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
		return rows;
	}
	case 'M': {
		static const Uint8 rows[] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
		return rows;
	}
	case 'N': {
		static const Uint8 rows[] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
		return rows;
	}
	case 'O': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
		return rows;
	}
	case 'P': {
		static const Uint8 rows[] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
		return rows;
	}
	case 'Q': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
		return rows;
	}
	case 'R': {
		static const Uint8 rows[] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
		return rows;
	}
	case 'S': {
		static const Uint8 rows[] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
		return rows;
	}
	case 'T': {
		static const Uint8 rows[] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
		return rows;
	}
	case 'U': {
		static const Uint8 rows[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
		return rows;
	}
	case 'V': {
		static const Uint8 rows[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
		return rows;
	}
	case 'W': {
		static const Uint8 rows[] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
		return rows;
	}
	case 'X': {
		static const Uint8 rows[] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
		return rows;
	}
	case 'Y': {
		static const Uint8 rows[] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
		return rows;
	}
	case 'Z': {
		static const Uint8 rows[] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
		return rows;
	}
	case '0': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
		return rows;
	}
	case '1': {
		static const Uint8 rows[] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
		return rows;
	}
	case '2': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
		return rows;
	}
	case '3': {
		static const Uint8 rows[] = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
		return rows;
	}
	case '4': {
		static const Uint8 rows[] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
		return rows;
	}
	case '5': {
		static const Uint8 rows[] = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E};
		return rows;
	}
	case '6': {
		static const Uint8 rows[] = {0x0F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E};
		return rows;
	}
	case '7': {
		static const Uint8 rows[] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
		return rows;
	}
	case '8': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
		return rows;
	}
	case '9': {
		static const Uint8 rows[] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x1E};
		return rows;
	}
	case '.': {
		static const Uint8 rows[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
		return rows;
	}
	case '-': {
		static const Uint8 rows[] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
		return rows;
	}
	case ':': {
		static const Uint8 rows[] = {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00};
		return rows;
	}
	case ' ': {
		static const Uint8 rows[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		return rows;
	}
	default: {
		static const Uint8 rows[] = {0x00, 0x1F, 0x01, 0x06, 0x04, 0x00, 0x04};
		return rows;
	}
	}
}

void build_font_pixels(Uint8 pixels[kFontTextureWidth * kFontTextureHeight]) {
	SDL_memset(pixels, 0, kFontTextureWidth * kFontTextureHeight);
	for (int ascii = 32; ascii < 96; ++ascii) {
		const int glyph_index = ascii - 32;
		const int cell_x = (glyph_index % kFontColumns) * kFontCellWidth;
		const int cell_y = (glyph_index / kFontColumns) * kFontCellHeight;
		const Uint8* rows = glyph_rows(static_cast<char>(ascii));
		for (int row = 0; row < 7; ++row) {
			for (int col = 0; col < 5; ++col) {
				if ((rows[row] & (1 << (4 - col))) == 0) {
					continue;
				}
				const int x = cell_x + 1 + col;
				const int y = cell_y + row;
				pixels[y * kFontTextureWidth + x] = 255;
			}
		}
	}
}

bool create_font_resources(renderer_t* renderer, overlay_pass_state_t* state) {
	Uint8 pixels[kFontTextureWidth * kFontTextureHeight];
	build_font_pixels(pixels);

	SDL_GPUTextureCreateInfo texture_info = {};
	texture_info.type = SDL_GPU_TEXTURETYPE_2D;
	texture_info.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	texture_info.width = kFontTextureWidth;
	texture_info.height = kFontTextureHeight;
	texture_info.layer_count_or_depth = 1;
	texture_info.num_levels = 1;
	texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	state->font_texture = SDL_CreateGPUTexture(renderer->device, &texture_info);
	if (!check_resource(state->font_texture, "create font texture")) {
		return false;
	}

	SDL_GPUTransferBufferCreateInfo transfer_info = {};
	transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_info.size = sizeof(pixels);
	SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(renderer->device, &transfer_info);
	if (!check_resource(transfer, "create font transfer buffer")) {
		return false;
	}
	void* mapped = SDL_MapGPUTransferBuffer(renderer->device, transfer, false);
	if (!check_resource(mapped, "map font transfer buffer")) {
		SDL_ReleaseGPUTransferBuffer(renderer->device, transfer);
		return false;
	}
	SDL_memcpy(mapped, pixels, sizeof(pixels));
	SDL_UnmapGPUTransferBuffer(renderer->device, transfer);

	SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(renderer->device);
	if (!check_resource(commands, "acquire font upload command buffer")) {
		SDL_ReleaseGPUTransferBuffer(renderer->device, transfer);
		return false;
	}
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(commands);
	if (!check_resource(copy_pass, "begin font copy pass")) {
		SDL_CancelGPUCommandBuffer(commands);
		SDL_ReleaseGPUTransferBuffer(renderer->device, transfer);
		return false;
	}
	SDL_GPUTextureTransferInfo source = {};
	source.transfer_buffer = transfer;
	source.pixels_per_row = kFontTextureWidth;
	source.rows_per_layer = kFontTextureHeight;
	SDL_GPUTextureRegion destination = {};
	destination.texture = state->font_texture;
	destination.w = kFontTextureWidth;
	destination.h = kFontTextureHeight;
	destination.d = 1;
	SDL_UploadToGPUTexture(copy_pass, &source, &destination, false);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(commands);
	SDL_ReleaseGPUTransferBuffer(renderer->device, transfer);

	SDL_GPUSamplerCreateInfo sampler_info = {};
	sampler_info.min_filter = SDL_GPU_FILTER_NEAREST;
	sampler_info.mag_filter = SDL_GPU_FILTER_NEAREST;
	sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	state->font_sampler = SDL_CreateGPUSampler(renderer->device, &sampler_info);
	return check_resource(state->font_sampler, "create font sampler");
}

void push_text_quad(overlay_text_vertex_t* vertices, Uint32* count, const float x, const float y,
					const float w, const float h, const float u0, const float v0, const float u1,
					const float v1, const float color[4]) {
	if (*count + 6 > kMaxTextVertices) {
		return;
	}
	overlay_text_vertex_t quad[6] = {
		{{x, y}, {u0, v0}, {color[0], color[1], color[2], color[3]}},
		{{x + w, y}, {u1, v0}, {color[0], color[1], color[2], color[3]}},
		{{x + w, y + h}, {u1, v1}, {color[0], color[1], color[2], color[3]}},
		{{x, y}, {u0, v0}, {color[0], color[1], color[2], color[3]}},
		{{x + w, y + h}, {u1, v1}, {color[0], color[1], color[2], color[3]}},
		{{x, y + h}, {u0, v1}, {color[0], color[1], color[2], color[3]}},
	};
	SDL_memcpy(vertices + *count, quad, sizeof(quad));
	*count += 6;
}

void build_text_vertices(const starter::DebugState* debug, const scene_t* scene, const float viewport_width,
						 const float viewport_height, overlay_text_vertex_t* vertices, Uint32* count) {
	*count = 0;
	auto push_text_line = [&](const char* text, float base_x, float base_y, const float color[4], float scale) {
		float x = base_x;
		for (const char* c = text; *c; ++c) {
			char ch = *c;
			if (ch >= 'a' && ch <= 'z') {
				ch = static_cast<char>(ch - ('a' - 'A'));
			}
			if (ch == '_') {
				ch = '-';
			}
			if (ch < 32 || ch >= 96) {
				ch = '?';
			}
			const int glyph_index = ch - 32;
			const float u0 = static_cast<float>((glyph_index % kFontColumns) * kFontCellWidth) /
							 static_cast<float>(kFontTextureWidth);
			const float v0 = static_cast<float>((glyph_index / kFontColumns) * kFontCellHeight) /
							 static_cast<float>(kFontTextureHeight);
			const float u1 = static_cast<float>((glyph_index % kFontColumns) * kFontCellWidth + kFontCellWidth) /
							 static_cast<float>(kFontTextureWidth);
			const float v1 = static_cast<float>((glyph_index / kFontColumns) * kFontCellHeight + kFontCellHeight) /
							 static_cast<float>(kFontTextureHeight);
			push_text_quad(vertices, count, x, base_y, kFontCellWidth * scale, kFontCellHeight * scale, u0, v0, u1,
						   v1, color);
			x += 6.0f * scale;
		}
	};

	if (debug && debug->enabled && debug->show_panel) {
		const float base_x = 28.0f;
		const float base_y = 26.0f;
		const float scale = 2.0f;
		for (int line = 0; line < debug->line_count; ++line) {
			const float y = base_y + line * 16.0f;
			float color[4] = {0.92f, 0.94f, 0.98f, 0.95f};
			if (line == 0) {
				color[0] = 0.94f;
				color[1] = 0.79f;
				color[2] = 0.43f;
			} else if (line == 3) {
				color[0] = 0.56f;
				color[1] = 0.88f;
				color[2] = 0.63f;
			}
			push_text_line(debug->lines[line], base_x, y, color, scale);
		}
	}

	if (!scene) {
		return;
	}
	const entity_t focused = scene_focused_interactable(scene);
	if (focused >= starter::config::kSceneMaxEntities || !scene->alive[focused] || !scene->has_interactable[focused]) {
		return;
	}
	const float prompt_color[4] = {0.98f, 0.92f, 0.76f, 0.98f};
	const char* prompt = scene->interactables[focused].prompt;
	const float prompt_scale = 2.25f;
	const float prompt_width = SDL_strlen(prompt) * 6.0f * prompt_scale;
	const float prompt_x = viewport_width * 0.5f - prompt_width * 0.5f;
	const float prompt_y = viewport_height * 0.5f + 36.0f;
	push_text_line(prompt, prompt_x, prompt_y, prompt_color, prompt_scale);
}

bool overlay_pass_init(renderer_t* renderer, render_pass_t* pass) {
	overlay_pass_state_t* state = static_cast<overlay_pass_state_t*>(SDL_calloc(1, sizeof(*state)));
	if (!state) {
		return false;
	}

	SDL_GPUShader* panel_vertex = nullptr;
	SDL_GPUShader* panel_fragment = nullptr;
	SDL_GPUShader* text_vertex = nullptr;
	SDL_GPUShader* text_fragment = nullptr;
	SDL_GPUColorTargetDescription color_target = {};
	SDL_GPUGraphicsPipelineCreateInfo panel_info = {};
	SDL_GPUGraphicsPipelineCreateInfo text_info = {};
	SDL_GPUVertexBufferDescription text_vertex_buffer = {};
	SDL_GPUVertexAttribute text_attributes[3] = {};
	SDL_GPUBufferCreateInfo buffer_info = {};
	SDL_GPUTransferBufferCreateInfo transfer_info = {};

	color_target.format = renderer->swapchain_format;
	color_target.blend_state.enable_blend = true;
	color_target.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	color_target.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	color_target.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	color_target.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	color_target.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	color_target.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

	panel_vertex =
		shader_library_load(&renderer->shaders, "fullscreen.vert", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0);
	panel_fragment =
		shader_library_load(&renderer->shaders, "overlay.frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0);
	if (!panel_vertex || !panel_fragment) {
		goto fail;
	}

	panel_info.vertex_shader = panel_vertex;
	panel_info.fragment_shader = panel_fragment;
	panel_info.target_info.num_color_targets = 1;
	panel_info.target_info.color_target_descriptions = &color_target;
	state->panel_pipeline = SDL_CreateGPUGraphicsPipeline(renderer->device, &panel_info);
	if (!check_resource(state->panel_pipeline, "create overlay panel pipeline")) {
		goto fail;
	}

	text_vertex =
		shader_library_load(&renderer->shaders, "debug_text.vert", SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
	text_fragment =
		shader_library_load(&renderer->shaders, "debug_text.frag", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 1);
	if (!text_vertex || !text_fragment) {
		goto fail;
	}

	text_vertex_buffer.slot = 0;
	text_vertex_buffer.pitch = sizeof(overlay_text_vertex_t);
	text_vertex_buffer.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	text_attributes[0].location = 0;
	text_attributes[0].buffer_slot = 0;
	text_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	text_attributes[0].offset = 0;
	text_attributes[1].location = 1;
	text_attributes[1].buffer_slot = 0;
	text_attributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	text_attributes[1].offset = sizeof(float) * 2;
	text_attributes[2].location = 2;
	text_attributes[2].buffer_slot = 0;
	text_attributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	text_attributes[2].offset = sizeof(float) * 4;

	text_info.vertex_shader = text_vertex;
	text_info.fragment_shader = text_fragment;
	text_info.target_info.num_color_targets = 1;
	text_info.target_info.color_target_descriptions = &color_target;
	text_info.vertex_input_state.vertex_buffer_descriptions = &text_vertex_buffer;
	text_info.vertex_input_state.num_vertex_buffers = 1;
	text_info.vertex_input_state.vertex_attributes = text_attributes;
	text_info.vertex_input_state.num_vertex_attributes = SDL_arraysize(text_attributes);
	state->text_pipeline = SDL_CreateGPUGraphicsPipeline(renderer->device, &text_info);
	if (!check_resource(state->text_pipeline, "create overlay text pipeline")) {
		goto fail;
	}

	buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	buffer_info.size = sizeof(overlay_text_vertex_t) * kMaxTextVertices;
	state->text_vertex_buffer = SDL_CreateGPUBuffer(renderer->device, &buffer_info);
	if (!check_resource(state->text_vertex_buffer, "create overlay text buffer")) {
		goto fail;
	}

	transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transfer_info.size = sizeof(overlay_text_vertex_t) * kMaxTextVertices;
	state->text_transfer_buffer = SDL_CreateGPUTransferBuffer(renderer->device, &transfer_info);
	if (!check_resource(state->text_transfer_buffer, "create overlay text transfer buffer")) {
		goto fail;
	}

	if (!create_font_resources(renderer, state)) {
		goto fail;
	}

	SDL_ReleaseGPUShader(renderer->device, panel_vertex);
	SDL_ReleaseGPUShader(renderer->device, panel_fragment);
	SDL_ReleaseGPUShader(renderer->device, text_vertex);
	SDL_ReleaseGPUShader(renderer->device, text_fragment);
	pass->state = state;
	return true;

fail:
	if (panel_vertex) {
		SDL_ReleaseGPUShader(renderer->device, panel_vertex);
	}
	if (panel_fragment) {
		SDL_ReleaseGPUShader(renderer->device, panel_fragment);
	}
	if (text_vertex) {
		SDL_ReleaseGPUShader(renderer->device, text_vertex);
	}
	if (text_fragment) {
		SDL_ReleaseGPUShader(renderer->device, text_fragment);
	}
	if (state) {
		if (state->panel_pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->panel_pipeline);
		}
		if (state->text_pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->text_pipeline);
		}
		if (state->font_texture) {
			SDL_ReleaseGPUTexture(renderer->device, state->font_texture);
		}
		if (state->font_sampler) {
			SDL_ReleaseGPUSampler(renderer->device, state->font_sampler);
		}
		if (state->text_vertex_buffer) {
			SDL_ReleaseGPUBuffer(renderer->device, state->text_vertex_buffer);
		}
		if (state->text_transfer_buffer) {
			SDL_ReleaseGPUTransferBuffer(renderer->device, state->text_transfer_buffer);
		}
		SDL_free(state);
	}
	return false;
}

void overlay_pass_resize(renderer_t* renderer, render_pass_t* pass, Uint32 width, Uint32 height) {
	(void)renderer;
	(void)pass;
	(void)width;
	(void)height;
}

void overlay_pass_execute(renderer_t* renderer, render_pass_t* pass, const frame_context_t* frame) {
	overlay_pass_state_t* state = static_cast<overlay_pass_state_t*>(pass->state);
	Uint32 text_vertex_count = 0;
	const entity_t focused = frame->scene ? scene_focused_interactable(frame->scene) : UINT32_MAX;
	const bool show_prompt = frame->scene && focused < starter::config::kSceneMaxEntities &&
							 frame->scene->alive[focused] && frame->scene->has_interactable[focused];
	const bool show_panel = frame->debug && frame->debug->enabled && frame->debug->show_panel;
	if (show_panel || show_prompt) {
		void* mapped = SDL_MapGPUTransferBuffer(renderer->device, state->text_transfer_buffer, false);
		if (check_resource(mapped, "map overlay text transfer buffer")) {
			build_text_vertices(frame->debug, frame->scene, static_cast<float>(renderer->window_width),
								static_cast<float>(renderer->window_height),
								static_cast<overlay_text_vertex_t*>(mapped), &text_vertex_count);
			SDL_UnmapGPUTransferBuffer(renderer->device, state->text_transfer_buffer);
			if (text_vertex_count > 0) {
				SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(frame->commands);
				if (check_resource(copy_pass, "begin overlay text copy pass")) {
					SDL_GPUTransferBufferLocation location = {.transfer_buffer = state->text_transfer_buffer};
					SDL_GPUBufferRegion region = {
						.buffer = state->text_vertex_buffer,
						.size = static_cast<Uint32>(sizeof(overlay_text_vertex_t) * text_vertex_count),
					};
					SDL_UploadToGPUBuffer(copy_pass, &location, &region, false);
					SDL_EndGPUCopyPass(copy_pass);
				}
			}
		}
	}

	overlay_uniforms_t panel_uniforms = {
		.viewport = {0, 0, static_cast<int>(renderer->window_width), static_cast<int>(renderer->window_height)},
		.debug_state = {frame->debug && frame->debug->enabled ? 1 : 0,
						frame->debug && frame->debug->show_panel ? 1 : 0,
						frame->debug ? frame->debug->line_count : 0, 0},
	};
	SDL_GPUColorTargetInfo color = {
		.texture = frame->swapchain_texture,
		.load_op = SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
	};
	SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(frame->commands, &color, 1, nullptr);
	if (!check_resource(render_pass, "begin overlay render pass")) {
		return;
	}

	SDL_BindGPUGraphicsPipeline(render_pass, state->panel_pipeline);
	SDL_PushGPUFragmentUniformData(frame->commands, 0, &panel_uniforms, sizeof(panel_uniforms));
	SDL_DrawGPUPrimitives(render_pass, 4, 1, 0, 0);

	if (text_vertex_count > 0) {
		overlay_text_uniforms_t text_uniforms = {
			.viewport = {0.0f, 0.0f, static_cast<float>(renderer->window_width),
						 static_cast<float>(renderer->window_height)},
		};
		SDL_GPUBufferBinding binding = {.buffer = state->text_vertex_buffer};
		SDL_GPUTextureSamplerBinding font_binding = {
			.texture = state->font_texture,
			.sampler = state->font_sampler,
		};
		SDL_BindGPUGraphicsPipeline(render_pass, state->text_pipeline);
		SDL_BindGPUVertexBuffers(render_pass, 0, &binding, 1);
		SDL_BindGPUFragmentSamplers(render_pass, 0, &font_binding, 1);
		SDL_PushGPUVertexUniformData(frame->commands, 0, &text_uniforms, sizeof(text_uniforms));
		SDL_DrawGPUPrimitives(render_pass, text_vertex_count, 1, 0, 0);
	}

	SDL_EndGPURenderPass(render_pass);
}

void overlay_pass_destroy(renderer_t* renderer, render_pass_t* pass) {
	overlay_pass_state_t* state = static_cast<overlay_pass_state_t*>(pass->state);
	if (!state) {
		return;
	}
	if (state->panel_pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->panel_pipeline);
	}
	if (state->text_pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(renderer->device, state->text_pipeline);
	}
	if (state->font_texture) {
		SDL_ReleaseGPUTexture(renderer->device, state->font_texture);
	}
	if (state->font_sampler) {
		SDL_ReleaseGPUSampler(renderer->device, state->font_sampler);
	}
	if (state->text_vertex_buffer) {
		SDL_ReleaseGPUBuffer(renderer->device, state->text_vertex_buffer);
	}
	if (state->text_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(renderer->device, state->text_transfer_buffer);
	}
	SDL_free(state);
	pass->state = nullptr;
}

} // namespace

render_pass_t render_pass_overlay_create(void) {
	return (render_pass_t){
		.name = "overlay",
		.init = overlay_pass_init,
		.resize = overlay_pass_resize,
		.execute = overlay_pass_execute,
		.destroy = overlay_pass_destroy,
	};
}
