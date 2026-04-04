#include "save.h"
#include "helpers.h"
#include <stdio.h>

bool save_load_player(const char* path, camera_t* camera) {
	assert(path);
	assert(camera);
	FILE* file = fopen(path, "r");
	if (!file) {
		return false;
	}

	int version = 0;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	bool have_position = false;
	bool have_rotation = false;

	char key[64];
	while (fscanf(file, "%63s", key) == 1) {
		if (SDL_strcmp(key, "version") == 0) {
			fscanf(file, "%d", &version);
		} else if (SDL_strcmp(key, "player_position") == 0) {
			have_position = fscanf(file, "%f %f %f", &x, &y, &z) == 3;
		} else if (SDL_strcmp(key, "player_rotation") == 0) {
			have_rotation = fscanf(file, "%f %f", &pitch, &yaw) == 2;
		}
	}
	fclose(file);

	if (version != 1 || !have_position || !have_rotation) {
		SDL_Log("Ignoring invalid save file at %s", path);
		return false;
	}

	camera_set_position(camera, x, y, z);
	camera_set_rotation(camera, pitch, yaw);
	return true;
}

bool save_write_player(const char* path, const camera_t* camera) {
	assert(path);
	assert(camera);
	FILE* file = fopen(path, "w");
	if (!file) {
		SDL_Log("Failed to open save file %s", path);
		return false;
	}

	float x;
	float y;
	float z;
	float pitch;
	float yaw;
	camera_get_position(camera, &x, &y, &z);
	camera_get_rotation(camera, &pitch, &yaw);

	fprintf(file, "version 1\n");
	fprintf(file, "player_position %.6f %.6f %.6f\n", x, y, z);
	fprintf(file, "player_rotation %.6f %.6f\n", pitch, yaw);
	fclose(file);
	return true;
}
