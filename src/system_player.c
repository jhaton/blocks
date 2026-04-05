#include "system_player.h"
#include "helpers.h"
#include <math.h>

static bool player_ready(const scene_t* scene, entity_t player) {
	return player < SCENE_MAX_ENTITIES && scene->alive[player] && scene->has_transform[player] &&
		   scene->has_player_controller[player] && scene->has_gravity[player] && scene->has_respawn[player];
}

void system_player_apply_spawn(scene_t* scene, entity_t player, camera_t* camera) {
	assert(scene);
	assert(camera);
	if (!player_ready(scene, player)) {
		return;
	}
	transform_component_t* transform = &scene->transforms[player];
	player_controller_component_t* controller = &scene->player_controllers[player];
	gravity_component_t* gravity = &scene->gravities[player];
	respawn_component_t* respawn = &scene->respawns[player];
	SDL_memcpy(transform->position, respawn->spawn, sizeof(transform->position));
	transform->position[1] = respawn->ground_y + controller->eye_height;
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
	gravity_component_t* gravity = &scene->gravities[player];
	player_controller_component_t* controller = &scene->player_controllers[player];
	respawn_component_t* respawn = &scene->respawns[player];
	camera_get_position(camera, &transform->position[0], &transform->position[1], &transform->position[2]);
	const float ground_height = respawn->ground_y + controller->eye_height;
	if (transform->position[1] <= ground_height) {
		transform->position[1] = ground_height;
		gravity->velocity_y = 0.0f;
		gravity->grounded = true;
	} else {
		gravity->grounded = false;
	}
}

void system_player_move(scene_t* scene, entity_t player, const camera_t* camera, const bool* keys,
						float seconds) {
	assert(scene);
	assert(camera);
	assert(keys);
	if (!player_ready(scene, player)) {
		return;
	}
	transform_component_t* transform = &scene->transforms[player];
	const player_controller_component_t* controller = &scene->player_controllers[player];
	float x = 0.0f;
	float z = 0.0f;
	if (keys[SDL_SCANCODE_W]) {
		z += 1.0f;
	}
	if (keys[SDL_SCANCODE_S]) {
		z -= 1.0f;
	}
	if (keys[SDL_SCANCODE_A]) {
		x -= 1.0f;
	}
	if (keys[SDL_SCANCODE_D]) {
		x += 1.0f;
	}
	float speed = controller->move_speed;
	if (keys[SDL_SCANCODE_LSHIFT]) {
		speed *= controller->fast_multiplier;
	}
	if (keys[SDL_SCANCODE_LCTRL]) {
		speed *= controller->slow_multiplier;
	}
	float pitch;
	float yaw;
	camera_get_rotation(camera, &pitch, &yaw);
	const float s = sinf(yaw);
	const float c = cosf(yaw);
	transform->position[0] += c * x * speed * seconds + s * z * speed * seconds;
	transform->position[2] += s * x * speed * seconds - c * z * speed * seconds;
	(void)pitch;
}

void system_player_apply_gravity(scene_t* scene, entity_t player, float seconds) {
	assert(scene);
	if (!player_ready(scene, player)) {
		return;
	}
	transform_component_t* transform = &scene->transforms[player];
	const player_controller_component_t* controller = &scene->player_controllers[player];
	gravity_component_t* gravity = &scene->gravities[player];
	const respawn_component_t* respawn = &scene->respawns[player];
	const float ground_height = respawn->ground_y + controller->eye_height;

	if (transform->position[1] > ground_height + EPSILON || !gravity->grounded) {
		gravity->velocity_y -= gravity->gravity * seconds;
		transform->position[1] += gravity->velocity_y * seconds;
	}
	if (transform->position[1] <= ground_height) {
		transform->position[1] = ground_height;
		gravity->velocity_y = 0.0f;
		gravity->grounded = true;
	} else {
		gravity->grounded = false;
	}
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
