#pragma once

#include "camera.h"
#include <stdbool.h>

bool save_load_player(const char* path, camera_t* camera);
bool save_write_player(const char* path, const camera_t* camera);
