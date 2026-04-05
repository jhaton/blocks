#pragma once

namespace starter {

struct FrameInput {
	bool quit_requested = false;
	bool capture_mouse_requested = false;
	bool release_mouse_requested = false;
	bool toggle_fullscreen_requested = false;
	bool save_requested = false;
	bool load_requested = false;
	bool toggle_debug_requested = false;
	bool toggle_debug_panel_requested = false;
	bool toggle_debug_colliders_requested = false;
	bool toggle_debug_player_requested = false;
	bool toggle_debug_axes_requested = false;
	bool toggle_debug_shadow_requested = false;

	bool move_forward = false;
	bool move_backward = false;
	bool move_left = false;
	bool move_right = false;
	bool move_fast = false;
	bool move_slow = false;
	bool jump_requested = false;

	float mouse_delta_x = 0.0f;
	float mouse_delta_y = 0.0f;
	int pixel_width = 0;
	int pixel_height = 0;
	bool pixel_size_changed = false;
};

} // namespace starter
