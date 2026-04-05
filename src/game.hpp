#pragma once

#include "camera.hpp"
#include "frame_input.hpp"
#include "scene.hpp"

namespace starter {

struct Game {
	scene_t scene;
	camera_t player_camera;
	float elapsed_seconds = 0.0f;

	bool init();
	void request_save() const;
	void request_load();
	void update(const FrameInput& input, float seconds);
};

} // namespace starter
