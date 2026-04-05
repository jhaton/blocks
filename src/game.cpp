#include "game.hpp"

#include "config.hpp"
#include "save.hpp"
#include "system_oscillator.hpp"
#include "system_player.hpp"
#include "system_spinner.hpp"

namespace starter {

bool Game::init() {
	scene_build_default(&scene);
	camera_init(&player_camera, CAMERA_TYPE_PERSPECTIVE);
	system_player_apply_spawn(&scene, scene_player(&scene), &player_camera);
	if (save_load_player(config::kSavePath.data(), &player_camera)) {
		system_player_capture_camera(&scene, scene_player(&scene), &player_camera);
	}
	system_player_sync_camera(&scene, scene_player(&scene), &player_camera);
	return true;
}

void Game::request_save() const {
	save_write_player(config::kSavePath.data(), &player_camera);
}

void Game::request_load() {
	if (!save_load_player(config::kSavePath.data(), &player_camera)) {
		system_player_apply_spawn(&scene, scene_player(&scene), &player_camera);
	} else {
		system_player_capture_camera(&scene, scene_player(&scene), &player_camera);
	}
	system_player_sync_camera(&scene, scene_player(&scene), &player_camera);
}

void Game::update(const FrameInput& input, float seconds) {
	elapsed_seconds += seconds;
	if (input.load_requested) {
		request_load();
	}
	if (input.save_requested) {
		request_save();
	}

	camera_rotate(&player_camera, -input.mouse_delta_y * config::kPlayerMouseSensitivity,
				  input.mouse_delta_x * config::kPlayerMouseSensitivity);

	system_player_move(&scene, scene_player(&scene), &player_camera, input, seconds);
	system_player_apply_gravity(&scene, scene_player(&scene), seconds);
	system_player_respawn(&scene, scene_player(&scene), &player_camera);
	system_spinner_update(&scene, seconds);
	system_oscillator_update(&scene, elapsed_seconds);
	system_player_sync_camera(&scene, scene_player(&scene), &player_camera);
	camera_update(&player_camera);
}

} // namespace starter
