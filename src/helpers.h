#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

#define EPSILON SDL_FLT_EPSILON
#define PI SDL_PI_F

#define clamp(x, a, b) SDL_clamp(x, a, b)
#define deg(rad) ((rad) * 180.0f / PI)
#define rad(deg) ((deg) * PI / 180.0f)

#define assert(e) SDL_assert(e)

bool check_resource(const void* resource, const char* name);
