#include "save.hpp"
#include "helpers.hpp"
#include <stdio.h>

bool save_load_game(const char* path, camera_t* camera, scene_t* scene) {
	assert(path);
	assert(camera);
	assert(scene);
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
	bool loaded_interaction_state = false;

	char key[64];
	while (fscanf(file, "%63s", key) == 1) {
		if (SDL_strcmp(key, "version") == 0) {
			fscanf(file, "%d", &version);
		} else if (SDL_strcmp(key, "player_position") == 0) {
			have_position = fscanf(file, "%f %f %f", &x, &y, &z) == 3;
		} else if (SDL_strcmp(key, "player_rotation") == 0) {
			have_rotation = fscanf(file, "%f %f", &pitch, &yaw) == 2;
		} else if (SDL_strcmp(key, "interactable") == 0) {
			char name[64];
			int active = 0;
			if (fscanf(file, "%63s %d", name, &active) == 2) {
				for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; ++entity) {
					if (!scene->alive[entity] || !scene->has_interactable[entity]) {
						continue;
					}
					if (SDL_strcmp(scene->names[entity], name) != 0) {
						continue;
					}
					scene->interactables[entity].active = active != 0;
					const entity_t linked = scene->interactables[entity].linked_entity;
					if (linked < starter::config::kSceneMaxEntities && scene->alive[linked] &&
						scene->has_sliding_door[linked]) {
						scene->sliding_doors[linked].target_open_amount = active != 0 ? 1.0f : 0.0f;
						scene->sliding_doors[linked].open_amount = scene->sliding_doors[linked].target_open_amount;
						for (int axis = 0; axis < 3; ++axis) {
							scene->transforms[linked].position[axis] =
								scene->sliding_doors[linked].closed_position[axis] +
								scene->sliding_doors[linked].open_offset[axis] *
									scene->sliding_doors[linked].open_amount;
						}
						if (scene->has_box_collider[linked]) {
							scene->box_colliders[linked].solid = scene->sliding_doors[linked].open_amount < 0.98f;
						}
					}
					loaded_interaction_state = true;
					break;
				}
			}
		}
	}
	fclose(file);

	if ((version != 1 && version != 2) || !have_position || !have_rotation) {
		SDL_Log("Ignoring invalid save file at %s", path);
		return false;
	}

	camera_set_position(camera, x, y, z);
	camera_set_rotation(camera, pitch, yaw);
	(void)loaded_interaction_state;
	return true;
}

bool save_write_game(const char* path, const camera_t* camera, const scene_t* scene) {
	assert(path);
	assert(camera);
	assert(scene);
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

	fprintf(file, "version 2\n");
	fprintf(file, "player_position %.6f %.6f %.6f\n", x, y, z);
	fprintf(file, "player_rotation %.6f %.6f\n", pitch, yaw);
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; ++entity) {
		if (!scene->alive[entity] || !scene->has_interactable[entity]) {
			continue;
		}
		fprintf(file, "interactable %s %d\n", scene->names[entity], scene->interactables[entity].active ? 1 : 0);
	}
	fclose(file);
	return true;
}
