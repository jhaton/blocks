#include "system_spinner.hpp"
#include "config.hpp"
#include "helpers.hpp"

void system_spinner_update(scene_t* scene, float seconds) {
	assert(scene);
	for (entity_t entity = 0; entity < starter::config::kSceneMaxEntities; entity++) {
		if (!scene->alive[entity] || !scene->has_transform[entity] || !scene->has_spinner[entity]) {
			continue;
		}
		transform_component_t* transform = &scene->transforms[entity];
		const spinner_component_t* spinner = &scene->spinners[entity];
		transform->rotation[1] += rad(spinner->degrees_per_second) * seconds;
	}
}
