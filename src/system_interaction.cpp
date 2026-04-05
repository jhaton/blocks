#include "system_interaction.hpp"

#include "helpers.hpp"

namespace {

bool ray_box_intersection(const float origin[3], const float direction[3], const float center[3],
						  const float half_extents[3], const float max_distance, float* distance_out) {
	float tmin = 0.0f;
	float tmax = max_distance;
	for (int axis = 0; axis < 3; ++axis) {
		const float min_bound = center[axis] - half_extents[axis];
		const float max_bound = center[axis] + half_extents[axis];
		if (SDL_fabsf(direction[axis]) <= EPSILON) {
			if (origin[axis] < min_bound || origin[axis] > max_bound) {
				return false;
			}
			continue;
		}

		const float inv = 1.0f / direction[axis];
		float t1 = (min_bound - origin[axis]) * inv;
		float t2 = (max_bound - origin[axis]) * inv;
		if (t1 > t2) {
			const float swap = t1;
			t1 = t2;
			t2 = swap;
		}
		tmin = SDL_max(tmin, t1);
		tmax = SDL_min(tmax, t2);
		if (tmin > tmax) {
			return false;
		}
	}

	if (distance_out) {
		*distance_out = tmin;
	}
	return tmax >= 0.0f;
}

} // namespace

void system_interaction_focus(scene_t* scene, const camera_t* camera) {
	assert(scene);
	assert(camera);

	scene->focused_interactable = UINT32_MAX;
	float origin[3];
	camera_get_position(camera, &origin[0], &origin[1], &origin[2]);
	float direction[3];
	camera_get_vector(camera, &direction[0], &direction[1], &direction[2]);

	float closest_distance = 1.0e30f;
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; ++entity) {
		if (!scene->alive[entity] || !scene->has_interactable[entity] || !scene->has_transform[entity]) {
			continue;
		}
		float half_extents[3] = {0.75f, 0.75f, 0.75f};
		if (scene->has_box_collider[entity]) {
			SDL_memcpy(half_extents, scene->box_colliders[entity].half_extents, sizeof(half_extents));
		} else {
			SDL_memcpy(half_extents, scene->transforms[entity].scale, sizeof(half_extents));
		}

		float distance = 0.0f;
		if (!ray_box_intersection(origin, direction, scene->transforms[entity].position, half_extents,
								  scene->interactables[entity].use_distance, &distance)) {
			continue;
		}
		if (distance < closest_distance) {
			closest_distance = distance;
			scene->focused_interactable = entity;
		}
	}
}

void system_interaction_use(scene_t* scene, const starter::FrameInput& input) {
	assert(scene);
	if (!input.use_requested) {
		return;
	}

	const entity_t entity = scene->focused_interactable;
	if (entity >= starter::config::kSceneMaxEntities || !scene->alive[entity] ||
		!scene->has_interactable[entity]) {
		return;
	}

	interactable_component_t& interactable = scene->interactables[entity];
	interactable.active = !interactable.active;
	if (interactable.linked_entity >= starter::config::kSceneMaxEntities ||
		!scene->alive[interactable.linked_entity]) {
		return;
	}
	if (scene->has_sliding_door[interactable.linked_entity]) {
		scene->sliding_doors[interactable.linked_entity].target_open_amount = interactable.active ? 1.0f : 0.0f;
	}
}
