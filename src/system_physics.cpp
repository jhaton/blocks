#include "system_physics.hpp"

#include "helpers.hpp"

namespace {

bool player_ready(const scene_t* scene, entity_t player) {
	return player < starter::config::kSceneMaxEntities && scene->alive[player] &&
		   scene->has_transform[player] && scene->has_character_body[player] &&
		   scene->has_gravity[player];
}

float player_min_x(const transform_component_t& transform, const character_body_component_t& body) {
	return transform.position[0] - body.radius;
}

float player_max_x(const transform_component_t& transform, const character_body_component_t& body) {
	return transform.position[0] + body.radius;
}

float player_min_y(const transform_component_t& transform, const character_body_component_t& body) {
	return transform.position[1] - body.eye_offset;
}

float player_max_y(const transform_component_t& transform, const character_body_component_t& body) {
	return player_min_y(transform, body) + body.height;
}

float player_min_z(const transform_component_t& transform, const character_body_component_t& body) {
	return transform.position[2] - body.radius;
}

float player_max_z(const transform_component_t& transform, const character_body_component_t& body) {
	return transform.position[2] + body.radius;
}

bool overlaps(const transform_component_t& player_transform, const character_body_component_t& body,
			  const transform_component_t& box_transform, const box_collider_component_t& collider) {
	const float box_min_x = box_transform.position[0] - collider.half_extents[0];
	const float box_max_x = box_transform.position[0] + collider.half_extents[0];
	const float box_min_y = box_transform.position[1] - collider.half_extents[1];
	const float box_max_y = box_transform.position[1] + collider.half_extents[1];
	const float box_min_z = box_transform.position[2] - collider.half_extents[2];
	const float box_max_z = box_transform.position[2] + collider.half_extents[2];

	return player_max_x(player_transform, body) > box_min_x &&
		   player_min_x(player_transform, body) < box_max_x &&
		   player_max_y(player_transform, body) > box_min_y &&
		   player_min_y(player_transform, body) < box_max_y &&
		   player_max_z(player_transform, body) > box_min_z &&
		   player_min_z(player_transform, body) < box_max_z;
}

void resolve_axis(scene_t* scene, entity_t player, int axis, float previous_value, bool* grounded) {
	transform_component_t& player_transform = scene->transforms[player];
	character_body_component_t& body = scene->character_bodies[player];
	gravity_component_t& gravity = scene->gravities[player];
	const float delta = player_transform.position[axis] - previous_value;
	if (SDL_fabsf(delta) <= EPSILON) {
		return;
	}

	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; ++entity) {
		if (entity == player || !scene->alive[entity] || !scene->has_transform[entity] ||
			!scene->has_box_collider[entity]) {
			continue;
		}
		const box_collider_component_t& collider = scene->box_colliders[entity];
		if (!collider.solid) {
			continue;
		}
		const transform_component_t& box_transform = scene->transforms[entity];
		if (!overlaps(player_transform, body, box_transform, collider)) {
			continue;
		}

		const float box_min_x = box_transform.position[0] - collider.half_extents[0];
		const float box_max_x = box_transform.position[0] + collider.half_extents[0];
		const float box_min_y = box_transform.position[1] - collider.half_extents[1];
		const float box_max_y = box_transform.position[1] + collider.half_extents[1];
		const float box_min_z = box_transform.position[2] - collider.half_extents[2];
		const float box_max_z = box_transform.position[2] + collider.half_extents[2];

		switch (axis) {
		case 0:
			if (delta > 0.0f) {
				player_transform.position[0] = box_min_x - body.radius;
			} else {
				player_transform.position[0] = box_max_x + body.radius;
			}
			break;
		case 1: {
			const float top_offset = body.height - body.eye_offset;
			const float previous_min_y = previous_value - body.eye_offset;
			const float previous_max_y = previous_min_y + body.height;
			if (delta > 0.0f) {
				if (previous_max_y > box_min_y + EPSILON) {
					continue;
				}
				player_transform.position[1] = box_min_y - top_offset;
			} else {
				if (previous_min_y < box_max_y - EPSILON) {
					continue;
				}
				player_transform.position[1] = box_max_y + body.eye_offset;
				*grounded = true;
			}
			gravity.velocity_y = 0.0f;
			break;
		}
		case 2:
			if (delta > 0.0f) {
				player_transform.position[2] = box_min_z - body.radius;
			} else {
				player_transform.position[2] = box_max_z + body.radius;
			}
			break;
		default:
			break;
		}
	}
}

} // namespace

void system_physics_begin_player_step(scene_t* scene, entity_t player) {
	assert(scene);
	if (!player_ready(scene, player)) {
		return;
	}
	character_body_component_t& body = scene->character_bodies[player];
	const transform_component_t& transform = scene->transforms[player];
	SDL_memcpy(body.previous_position, transform.position, sizeof(body.previous_position));
}

void system_physics_simulate_player(scene_t* scene, entity_t player, bool jump_requested, float seconds) {
	assert(scene);
	if (!player_ready(scene, player)) {
		return;
	}

	transform_component_t& transform = scene->transforms[player];
	const character_body_component_t& body = scene->character_bodies[player];
	gravity_component_t& gravity = scene->gravities[player];
	bool grounded = false;

	if (jump_requested && gravity.grounded) {
		gravity.velocity_y = body.jump_speed;
		gravity.grounded = false;
	}

	if (!gravity.grounded || gravity.velocity_y > 0.0f) {
		gravity.velocity_y -= gravity.gravity * seconds;
		transform.position[1] += gravity.velocity_y * seconds;
	} else {
		gravity.velocity_y = 0.0f;
	}

	resolve_axis(scene, player, 1, body.previous_position[1], &grounded);
	resolve_axis(scene, player, 0, body.previous_position[0], &grounded);
	resolve_axis(scene, player, 2, body.previous_position[2], &grounded);
	gravity.grounded = grounded;
}
