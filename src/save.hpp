#pragma once

#include "camera.hpp"
#include "scene.hpp"
#include <stdbool.h>

bool save_load_game(const char* path, camera_t* camera, scene_t* scene);
bool save_write_game(const char* path, const camera_t* camera, const scene_t* scene);
