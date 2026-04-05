#pragma once

#include "config.h"
#include <stdbool.h>
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
	bool alive[SCENE_MAX_ENTITIES];
	char names[SCENE_MAX_ENTITIES][32];

	bool has_transform[SCENE_MAX_ENTITIES];
	transform_component_t transforms[SCENE_MAX_ENTITIES];

	bool has_renderable[SCENE_MAX_ENTITIES];
	renderable_component_t renderables[SCENE_MAX_ENTITIES];

	bool has_light[SCENE_MAX_ENTITIES];
	directional_light_component_t lights[SCENE_MAX_ENTITIES];

	bool has_oscillator[SCENE_MAX_ENTITIES];
	oscillator_component_t oscillators[SCENE_MAX_ENTITIES];

	bool has_spinner[SCENE_MAX_ENTITIES];
	spinner_component_t spinners[SCENE_MAX_ENTITIES];

	bool has_player_controller[SCENE_MAX_ENTITIES];
	player_controller_component_t player_controllers[SCENE_MAX_ENTITIES];

	bool has_gravity[SCENE_MAX_ENTITIES];
	gravity_component_t gravities[SCENE_MAX_ENTITIES];

	bool has_respawn[SCENE_MAX_ENTITIES];
	respawn_component_t respawns[SCENE_MAX_ENTITIES];

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
gravity_component_t* scene_add_gravity(scene_t* scene, entity_t entity);
respawn_component_t* scene_add_respawn(scene_t* scene, entity_t entity);
const directional_light_component_t* scene_main_light(const scene_t* scene);
entity_t scene_player(const scene_t* scene);
void scene_update(scene_t* scene, float seconds);
void scene_build_default(scene_t* scene);
