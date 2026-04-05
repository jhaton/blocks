#pragma once

#include "config.hpp"
#include <stdint.h>

typedef uint32_t entity_t;

typedef enum {
	MESH_BOX,
	MESH_COUNT,
} mesh_type_t;

typedef struct {
	float position[3];
	float rotation[3];
	float scale[3];
} transform_component_t;

typedef struct {
	mesh_type_t mesh;
	float color[3];
	bool cast_shadow;
} renderable_component_t;

typedef struct {
	float direction[3];
	float color[3];
	float intensity;
} directional_light_component_t;

typedef struct {
	float origin[3];
	float axis[3];
	float amplitude;
	float speed;
	float phase;
} oscillator_component_t;

typedef struct {
	float degrees_per_second;
} spinner_component_t;

typedef struct {
	float move_speed;
	float fast_multiplier;
	float slow_multiplier;
	float eye_height;
} player_controller_component_t;

typedef struct {
	float radius;
	float height;
	float eye_offset;
	float jump_speed;
	float previous_position[3];
} character_body_component_t;

typedef struct {
	float velocity_y;
	float gravity;
	bool grounded;
} gravity_component_t;

typedef struct {
	float spawn[3];
	float ground_y;
	float reset_y;
} respawn_component_t;

typedef struct {
	float half_extents[3];
	bool solid;
} box_collider_component_t;

typedef struct {
	bool alive[starter::config::kSceneMaxEntities];
	char names[starter::config::kSceneMaxEntities][32];

	bool has_transform[starter::config::kSceneMaxEntities];
	transform_component_t transforms[starter::config::kSceneMaxEntities];

	bool has_renderable[starter::config::kSceneMaxEntities];
	renderable_component_t renderables[starter::config::kSceneMaxEntities];

	bool has_light[starter::config::kSceneMaxEntities];
	directional_light_component_t lights[starter::config::kSceneMaxEntities];

	bool has_oscillator[starter::config::kSceneMaxEntities];
	oscillator_component_t oscillators[starter::config::kSceneMaxEntities];

	bool has_spinner[starter::config::kSceneMaxEntities];
	spinner_component_t spinners[starter::config::kSceneMaxEntities];

	bool has_player_controller[starter::config::kSceneMaxEntities];
	player_controller_component_t player_controllers[starter::config::kSceneMaxEntities];

	bool has_character_body[starter::config::kSceneMaxEntities];
	character_body_component_t character_bodies[starter::config::kSceneMaxEntities];

	bool has_gravity[starter::config::kSceneMaxEntities];
	gravity_component_t gravities[starter::config::kSceneMaxEntities];

	bool has_respawn[starter::config::kSceneMaxEntities];
	respawn_component_t respawns[starter::config::kSceneMaxEntities];

	bool has_box_collider[starter::config::kSceneMaxEntities];
	box_collider_component_t box_colliders[starter::config::kSceneMaxEntities];

	entity_t sun;
	entity_t player;
	uint32_t count;
} scene_t;

void scene_init(scene_t* scene);
entity_t scene_create(scene_t* scene, const char* name);
transform_component_t* scene_add_transform(scene_t* scene, entity_t entity);
renderable_component_t* scene_add_renderable(scene_t* scene, entity_t entity);
directional_light_component_t* scene_add_directional_light(scene_t* scene, entity_t entity);
oscillator_component_t* scene_add_oscillator(scene_t* scene, entity_t entity);
spinner_component_t* scene_add_spinner(scene_t* scene, entity_t entity);
player_controller_component_t* scene_add_player_controller(scene_t* scene, entity_t entity);
character_body_component_t* scene_add_character_body(scene_t* scene, entity_t entity);
gravity_component_t* scene_add_gravity(scene_t* scene, entity_t entity);
respawn_component_t* scene_add_respawn(scene_t* scene, entity_t entity);
box_collider_component_t* scene_add_box_collider(scene_t* scene, entity_t entity);
const directional_light_component_t* scene_main_light(const scene_t* scene);
entity_t scene_player(const scene_t* scene);
void scene_build_default(scene_t* scene);
