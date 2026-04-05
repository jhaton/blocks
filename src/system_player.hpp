#pragma once

#include "camera.hpp"
#include "frame_input.hpp"
#include "scene.hpp"

void system_player_apply_spawn(scene_t* scene, entity_t player, camera_t* camera);
void system_player_capture_camera(scene_t* scene, entity_t player, const camera_t* camera);
void system_player_move(scene_t* scene, entity_t player, const camera_t* camera, const starter::FrameInput& input,
						float seconds);
void system_player_apply_gravity(scene_t* scene, entity_t player, float seconds);
void system_player_respawn(scene_t* scene, entity_t player, camera_t* camera);
void system_player_sync_camera(const scene_t* scene, entity_t player, camera_t* camera);
