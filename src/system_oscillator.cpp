#include "system_oscillator.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include <math.h>

void system_oscillator_update(scene_t* scene, float elapsed_seconds) {
	assert(scene);
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; entity++) {
		if (!scene->alive[entity] || !scene->has_transform[entity] || !scene->has_oscillator[entity]) {
			continue;
		}
		transform_component_t* transform = &scene->transforms[entity];
		const oscillator_component_t* oscillator = &scene->oscillators[entity];
		const float wave = sinf(elapsed_seconds * oscillator->speed + oscillator->phase);
		for (int i = 0; i < 3; i++) {
			transform->position[i] =
				oscillator->origin[i] + oscillator->axis[i] * oscillator->amplitude * wave;
		}
	}
}
