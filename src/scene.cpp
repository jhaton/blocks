#include "scene.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include "math3d.hpp"

static void set_default_transform(transform_component_t* transform) {
	assert(transform);
	math3d_vec3_set(transform->position, 0.0f, 0.0f, 0.0f);
	math3d_vec3_set(transform->rotation, 0.0f, 0.0f, 0.0f);
	math3d_vec3_set(transform->scale, 1.0f, 1.0f, 1.0f);
}

void scene_init(scene_t* scene) {
	assert(scene);
	SDL_zero(*scene);
	scene->sun = UINT32_MAX;
	scene->player = UINT32_MAX;
}

entity_t scene_create(scene_t* scene, const char* name) {
	assert(scene);
	assert(name);
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; entity++) {
		if (scene->alive[entity]) {
			continue;
		}
		scene->alive[entity] = true;
		SDL_strlcpy(scene->names[entity], name, sizeof(scene->names[entity]));
		scene->count++;
		return entity;
	}
	SDL_Log("Scene capacity reached");
	return UINT32_MAX;
}

transform_component_t* scene_add_transform(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_transform[entity] = true;
	set_default_transform(&scene->transforms[entity]);
	return &scene->transforms[entity];
}

renderable_component_t* scene_add_renderable(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_renderable[entity] = true;
	renderable_component_t* renderable = &scene->renderables[entity];
	renderable->mesh = MESH_BOX;
	math3d_vec3_set(renderable->color, 0.8f, 0.8f, 0.8f);
	renderable->cast_shadow = true;
	return renderable;
}

directional_light_component_t* scene_add_directional_light(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_light[entity] = true;
	scene->sun = entity;
	directional_light_component_t* light = &scene->lights[entity];
	math3d_vec3_set(light->direction, -0.45f, -1.0f, -0.25f);
	math3d_vec3_normalize(light->direction);
	math3d_vec3_set(light->color, 1.0f, 0.95f, 0.82f);
	light->intensity = 1.2f;
	return light;
}

oscillator_component_t* scene_add_oscillator(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_oscillator[entity] = true;
	oscillator_component_t* oscillator = &scene->oscillators[entity];
	math3d_vec3_set(oscillator->origin, 0.0f, 0.0f, 0.0f);
	math3d_vec3_set(oscillator->axis, 0.0f, 1.0f, 0.0f);
	oscillator->amplitude = 1.0f;
	oscillator->speed = 1.0f;
	oscillator->phase = 0.0f;
	return oscillator;
}

spinner_component_t* scene_add_spinner(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_spinner[entity] = true;
	spinner_component_t* spinner = &scene->spinners[entity];
	spinner->degrees_per_second = 35.0f;
	return spinner;
}

player_controller_component_t* scene_add_player_controller(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_player_controller[entity] = true;
	player_controller_component_t* controller = &scene->player_controllers[entity];
	controller->move_speed = starter::config::kPlayerMoveSpeed;
	controller->fast_multiplier = starter::config::kPlayerFastMultiplier;
	controller->slow_multiplier = starter::config::kPlayerSlowMultiplier;
	controller->eye_height = starter::config::kPlayerHeight;
	scene->player = entity;
	return controller;
}

character_body_component_t* scene_add_character_body(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_character_body[entity] = true;
	character_body_component_t* body = &scene->character_bodies[entity];
	body->radius = starter::config::kPlayerRadius;
	body->height = starter::config::kPlayerHeight;
	body->eye_offset = starter::config::kPlayerHeight;
	body->jump_speed = starter::config::kPlayerJumpSpeed;
	math3d_vec3_set(body->previous_position, 0.0f, 0.0f, 0.0f);
	return body;
}

gravity_component_t* scene_add_gravity(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_gravity[entity] = true;
	gravity_component_t* gravity = &scene->gravities[entity];
	gravity->velocity_y = 0.0f;
	gravity->gravity = starter::config::kPlayerGravity;
	gravity->grounded = true;
	return gravity;
}

respawn_component_t* scene_add_respawn(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_respawn[entity] = true;
	respawn_component_t* respawn = &scene->respawns[entity];
	math3d_vec3_set(respawn->spawn, 0.0f, starter::config::kPlayerHeight, 28.0f);
	respawn->ground_y = starter::config::kPlayerGroundY;
	respawn->reset_y = starter::config::kPlayerResetY;
	return respawn;
}

box_collider_component_t* scene_add_box_collider(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < starter::config::kSceneMaxEntities);
	scene->has_box_collider[entity] = true;
	box_collider_component_t* collider = &scene->box_colliders[entity];
	math3d_vec3_set(collider->half_extents, 0.5f, 0.5f, 0.5f);
	collider->solid = true;
	return collider;
}

const directional_light_component_t* scene_main_light(const scene_t* scene) {
	assert(scene);
	if (scene->sun >= starter::config::kSceneMaxEntities || !scene->has_light[scene->sun]) {
		return NULL;
	}
	return &scene->lights[scene->sun];
}

entity_t scene_player(const scene_t* scene) {
	assert(scene);
	return scene->player;
}

static entity_t add_box(scene_t* scene, const char* name, const float position[3], const float scale[3],
						const float color[3]) {
	entity_t entity = scene_create(scene, name);
	transform_component_t* transform = scene_add_transform(scene, entity);
	renderable_component_t* renderable = scene_add_renderable(scene, entity);
	box_collider_component_t* collider = scene_add_box_collider(scene, entity);
	SDL_memcpy(transform->position, position, sizeof(transform->position));
	SDL_memcpy(transform->scale, scale, sizeof(transform->scale));
	SDL_memcpy(renderable->color, color, sizeof(renderable->color));
	SDL_memcpy(collider->half_extents, scale, sizeof(collider->half_extents));
	return entity;
}

void scene_build_default(scene_t* scene) {
	assert(scene);
	scene_init(scene);

	const float slate[] = {0.16f, 0.19f, 0.24f};
	const float sand[] = {0.72f, 0.69f, 0.58f};
	const float moss[] = {0.39f, 0.56f, 0.42f};
	const float coral[] = {0.82f, 0.47f, 0.36f};
	const float sky[] = {0.60f, 0.70f, 0.84f};

	entity_t sun = scene_create(scene, "sun");
	scene_add_directional_light(scene, sun);

	entity_t player = scene_create(scene, "player");
	transform_component_t* player_transform = scene_add_transform(scene, player);
	player_controller_component_t* controller = scene_add_player_controller(scene, player);
	character_body_component_t* body = scene_add_character_body(scene, player);
	gravity_component_t* gravity = scene_add_gravity(scene, player);
	respawn_component_t* respawn = scene_add_respawn(scene, player);
	math3d_vec3_copy(player_transform->position, respawn->spawn);
	math3d_vec3_copy(body->previous_position, player_transform->position);
	gravity->grounded = true;

	const float floor_position[3] = {0.0f, -0.5f, 0.0f};
	const float floor_scale[3] = {60.0f, 1.0f, 60.0f};
	const float back_wall_position[3] = {0.0f, 4.0f, -60.0f};
	const float back_wall_scale[3] = {60.0f, 9.0f, 1.0f};
	const float front_wall_position[3] = {0.0f, 4.0f, 60.0f};
	const float front_wall_scale[3] = {60.0f, 9.0f, 1.0f};
	const float left_wall_position[3] = {-60.0f, 4.0f, 0.0f};
	const float left_wall_scale[3] = {1.0f, 9.0f, 60.0f};
	const float right_wall_position[3] = {60.0f, 4.0f, 0.0f};
	const float right_wall_scale[3] = {1.0f, 9.0f, 60.0f};
	const float platform_a_position[3] = {-20.0f, 1.4f, -12.0f};
	const float platform_a_scale[3] = {8.0f, 0.8f, 8.0f};
	const float platform_b_position[3] = {18.0f, 2.0f, 15.0f};
	const float platform_b_scale[3] = {7.0f, 0.8f, 12.0f};
	const float moving_position[3] = {0.0f, 3.0f, -26.0f};
	const float moving_scale[3] = {5.0f, 0.5f, 5.0f};
	const float monolith_position[3] = {0.0f, 5.0f, 0.0f};
	const float monolith_scale[3] = {4.0f, 10.0f, 4.0f};
	const float beacon_position[3] = {24.0f, 6.5f, -24.0f};
	const float beacon_scale[3] = {2.0f, 13.0f, 2.0f};
	const float cover1_position[3] = {-24.0f, 2.5f, 20.0f};
	const float cover1_scale[3] = {5.0f, 5.0f, 5.0f};
	const float cover2_position[3] = {-8.0f, 1.8f, 30.0f};
	const float cover2_scale[3] = {3.0f, 3.0f, 10.0f};
	const float cover3_position[3] = {28.0f, 2.2f, 22.0f};
	const float cover3_scale[3] = {8.0f, 4.0f, 4.0f};

	add_box(scene, "floor", floor_position, floor_scale, sand);
	add_box(scene, "back_wall", back_wall_position, back_wall_scale, slate);
	add_box(scene, "front_wall", front_wall_position, front_wall_scale, slate);
	add_box(scene, "left_wall", left_wall_position, left_wall_scale, slate);
	add_box(scene, "right_wall", right_wall_position, right_wall_scale, slate);

	add_box(scene, "platform_a", platform_a_position, platform_a_scale, moss);
	add_box(scene, "platform_b", platform_b_position, platform_b_scale, sky);

	entity_t moving = add_box(scene, "moving_block", moving_position, moving_scale, coral);
	oscillator_component_t* oscillator = scene_add_oscillator(scene, moving);
	math3d_vec3_set(oscillator->origin, 0.0f, 3.0f, -26.0f);
	math3d_vec3_set(oscillator->axis, 1.0f, 0.0f, 0.0f);
	oscillator->amplitude = 18.0f;
	oscillator->speed = 0.8f;
	oscillator->phase = rad(45.0f);

	entity_t monolith = add_box(scene, "monolith", monolith_position, monolith_scale, slate);
	scene_add_spinner(scene, monolith)->degrees_per_second = 18.0f;

	entity_t beacon = add_box(scene, "beacon", beacon_position, beacon_scale, sky);
	scene_add_spinner(scene, beacon)->degrees_per_second = -26.0f;

	add_box(scene, "cover_1", cover1_position, cover1_scale, moss);
	add_box(scene, "cover_2", cover2_position, cover2_scale, coral);
	add_box(scene, "cover_3", cover3_position, cover3_scale, sky);
}
