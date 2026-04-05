#include "system_door.hpp"

#include "helpers.hpp"

void system_door_update(scene_t* scene, float seconds) {
	assert(scene);
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; ++entity) {
		if (!scene->alive[entity] || !scene->has_transform[entity] || !scene->has_sliding_door[entity]) {
			continue;
		}

		sliding_door_component_t& door = scene->sliding_doors[entity];
		if (door.open_amount < door.target_open_amount) {
			door.open_amount = SDL_min(door.target_open_amount, door.open_amount + door.speed * seconds);
		} else if (door.open_amount > door.target_open_amount) {
			door.open_amount = SDL_max(door.target_open_amount, door.open_amount - door.speed * seconds);
		}

		transform_component_t& transform = scene->transforms[entity];
		for (int axis = 0; axis < 3; ++axis) {
			transform.position[axis] = door.closed_position[axis] + door.open_offset[axis] * door.open_amount;
		}

		if (scene->has_box_collider[entity]) {
			scene->box_colliders[entity].solid = door.open_amount < 0.98f;
		}
	}
}
