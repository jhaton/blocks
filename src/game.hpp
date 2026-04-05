#pragma once

#include "camera.hpp"
#include "debug.hpp"
#include "frame_input.hpp"
#include "scene.hpp"

namespace starter {

struct Game {
	scene_t scene;
	camera_t player_camera;
	DebugState debug;
	float elapsed_seconds = 0.0f;

	bool init();
	void request_save() const;
	void request_load();
	void update(const FrameInput& input, float seconds);
};

} // namespace starter
