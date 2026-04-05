#pragma once

#include "scene.hpp"

void system_physics_begin_player_step(scene_t* scene, entity_t player);
void system_physics_simulate_player(scene_t* scene, entity_t player, bool jump_requested, float seconds);
