#include "system_player.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include <cmath>

static bool player_ready(const scene_t* scene, entity_t player) {
	return player < starter::config::kSceneMaxEntities && scene->alive[player] &&
		   scene->has_transform[player] && scene->has_player_controller[player] &&
		   scene->has_character_body[player] &&
		   scene->has_gravity[player] && scene->has_respawn[player];
}

void system_player_apply_spawn(scene_t* scene, entity_t player, camera_t* camera) {
	assert(scene);
	assert(camera);
	if (!player_ready(scene, player)) {
		return;
	}
	transform_component_t* transform = &scene->transforms[player];
	player_controller_component_t* controller = &scene->player_controllers[player];
	character_body_component_t* body = &scene->character_bodies[player];
	gravity_component_t* gravity = &scene->gravities[player];
	respawn_component_t* respawn = &scene->respawns[player];
	SDL_memcpy(transform->position, respawn->spawn, sizeof(transform->position));
	SDL_memcpy(body->previous_position, transform->position, sizeof(body->previous_position));
	gravity->velocity_y = 0.0f;
	gravity->grounded = true;
	camera_set_position(camera, transform->position[0], transform->position[1], transform->position[2]);
	camera_set_rotation(camera, rad(-6.0f), rad(0.0f));
}

void system_player_capture_camera(scene_t* scene, entity_t player, const camera_t* camera) {
	assert(scene);
	assert(camera);
	if (!player_ready(scene, player)) {
		return;
	}
	transform_component_t* transform = &scene->transforms[player];
	character_body_component_t* body = &scene->character_bodies[player];
	gravity_component_t* gravity = &scene->gravities[player];
	camera_get_position(camera, &transform->position[0], &transform->position[1], &transform->position[2]);
	SDL_memcpy(body->previous_position, transform->position, sizeof(body->previous_position));
	gravity->velocity_y = 0.0f;
}

void system_player_move(scene_t* scene, entity_t player, const camera_t* camera, const starter::FrameInput& input,
						float seconds) {
	assert(scene);
	assert(camera);
	if (!player_ready(scene, player)) {
		return;
	}
	transform_component_t* transform = &scene->transforms[player];
	const player_controller_component_t* controller = &scene->player_controllers[player];
	float x = 0.0f;
	float z = 0.0f;
	if (input.move_forward) {
		z += 1.0f;
	}
	if (input.move_backward) {
		z -= 1.0f;
	}
	if (input.move_left) {
		x -= 1.0f;
	}
	if (input.move_right) {
		x += 1.0f;
	}
	float speed = controller->move_speed;
	if (input.move_fast) {
		speed *= controller->fast_multiplier;
	}
	if (input.move_slow) {
		speed *= controller->slow_multiplier;
	}
	float pitch;
	float yaw;
	camera_get_rotation(camera, &pitch, &yaw);
	const float s = std::sinf(yaw);
	const float c = std::cosf(yaw);
	transform->position[0] += c * x * speed * seconds + s * z * speed * seconds;
	transform->position[2] += s * x * speed * seconds - c * z * speed * seconds;
	(void)pitch;
}

void system_player_respawn(scene_t* scene, entity_t player, camera_t* camera) {
	assert(scene);
	assert(camera);
	if (!player_ready(scene, player)) {
		return;
	}
	const transform_component_t* transform = &scene->transforms[player];
	const respawn_component_t* respawn = &scene->respawns[player];
	if (transform->position[1] < respawn->reset_y) {
		system_player_apply_spawn(scene, player, camera);
	}
}

void system_player_sync_camera(const scene_t* scene, entity_t player, camera_t* camera) {
	assert(scene);
	assert(camera);
	if (!player_ready(scene, player)) {
		return;
	}
	const transform_component_t* transform = &scene->transforms[player];
	camera_set_position(camera, transform->position[0], transform->position[1], transform->position[2]);
}
