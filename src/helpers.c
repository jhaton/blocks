#include "helpers.h"

bool check_resource(const void* resource, const char* name) {
	if (!resource) {
		SDL_Log("Failed to %s: %s", name, SDL_GetError());
		return false;
	}
	return true;
}
