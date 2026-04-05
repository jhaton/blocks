#include "debug.hpp"

#include "helpers.hpp"

namespace starter {

namespace {

void set_line(DebugState* debug, const int index, const char* format, ...) {
	assert(debug);
	assert(index >= 0);
	assert(index < kDebugLineCount);
	va_list args;
	va_start(args, format);
	SDL_vsnprintf(debug->lines[index], kDebugLineLength, format, args);
	va_end(args);
}

} // namespace

void debug_init(DebugState* debug) {
	assert(debug);
	*debug = {};
	debug->show_panel = true;
	debug->draw_player_body = true;
}

void debug_handle_input(DebugState* debug, const FrameInput& input) {
	assert(debug);
	if (input.toggle_debug_requested) {
		debug->enabled = !debug->enabled;
	}
	if (input.toggle_debug_panel_requested) {
		debug->show_panel = !debug->show_panel;
	}
	if (input.toggle_debug_colliders_requested) {
		debug->draw_colliders = !debug->draw_colliders;
	}
	if (input.toggle_debug_player_requested) {
		debug->draw_player_body = !debug->draw_player_body;
	}
	if (input.toggle_debug_axes_requested) {
		debug->draw_axes = !debug->draw_axes;
	}
	if (input.toggle_debug_shadow_requested) {
		debug->draw_shadow = !debug->draw_shadow;
	}
}

void debug_update(DebugState* debug, const scene_t* scene, const camera_t* camera, float seconds) {
	assert(debug);
	assert(scene);
	assert(camera);

	debug->line_count = 0;
	debug->entity_count = static_cast<int>(scene->count);
	if (seconds > EPSILON) {
		debug->fps = 1.0f / seconds;
		debug->frame_ms = seconds * 1000.0f;
	}

	if (!debug->enabled || !debug->show_panel) {
		return;
	}

	const entity_t player = scene_player(scene);
	float velocity_y = 0.0f;
	bool grounded = false;
	if (player < starter::config::kSceneMaxEntities && scene->alive[player] && scene->has_gravity[player]) {
		velocity_y = scene->gravities[player].velocity_y;
		grounded = scene->gravities[player].grounded;
	}

	set_line(debug, debug->line_count++, "FPS %03d  FRAME %.2fMS", static_cast<int>(debug->fps + 0.5f),
			 debug->frame_ms);
	set_line(debug, debug->line_count++, "PLAYER POS %.1f %.1f %.1f", camera->x, camera->y, camera->z);
	set_line(debug, debug->line_count++, "PITCH %.1f  YAW %.1f", deg(camera->pitch), deg(camera->yaw));
	set_line(debug, debug->line_count++, "VELY %.2f  GROUND %d", velocity_y, grounded ? 1 : 0);
	set_line(debug, debug->line_count++, "ENTITIES %d  PASSES %d", debug->entity_count, debug->render_pass_count);
	set_line(debug, debug->line_count++, "F1 DEBUG  F2 PANEL  F3 COLL");
	set_line(debug, debug->line_count++, "F4 BODY   F6 AXES   F7 SHADOW");
	set_line(debug, debug->line_count++, "COL %d BODY %d AXES %d SHADOW %d",
			 debug->draw_colliders ? 1 : 0, debug->draw_player_body ? 1 : 0,
			 debug->draw_axes ? 1 : 0, debug->draw_shadow ? 1 : 0);
}

void debug_set_renderer_info(DebugState* debug, const camera_t* shadow_camera, int render_pass_count) {
	assert(debug);
	assert(shadow_camera);
	debug->render_pass_count = render_pass_count;
	if (!debug->enabled || !debug->show_panel) {
		return;
	}
	if (debug->line_count < kDebugLineCount) {
		set_line(debug, debug->line_count++, "SHADOW POS %.1f %.1f %.1f", shadow_camera->x, shadow_camera->y,
				 shadow_camera->z);
	}
	if (debug->line_count < kDebugLineCount) {
		set_line(debug, debug->line_count++, "SHADOW PITCH %.1f YAW %.1f", deg(shadow_camera->pitch),
				 deg(shadow_camera->yaw));
	}
}

} // namespace starter
