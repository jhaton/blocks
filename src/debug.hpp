#pragma once

#include "camera.hpp"
#include "frame_input.hpp"
#include "scene.hpp"

namespace starter {

inline constexpr int kDebugLineCount = 10;
inline constexpr int kDebugLineLength = 96;

struct DebugState {
	bool enabled = false;
	bool show_panel = true;
	bool draw_colliders = false;
	bool draw_player_body = true;
	bool draw_axes = false;
	bool draw_shadow = false;

	float fps = 0.0f;
	float frame_ms = 0.0f;
	int entity_count = 0;
	int render_pass_count = 0;
	char lines[kDebugLineCount][kDebugLineLength] = {};
	int line_count = 0;
};

void debug_init(DebugState* debug);
void debug_handle_input(DebugState* debug, const FrameInput& input);
void debug_update(DebugState* debug, const scene_t* scene, const camera_t* camera, float seconds);
void debug_set_renderer_info(DebugState* debug, const camera_t* shadow_camera, int render_pass_count);

} // namespace starter
