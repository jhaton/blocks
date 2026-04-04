#include "scene.h"
#include "helpers.h"
#include "math3d.h"
#include <math.h>

static void set_default_transform(transform_component_t* transform) {
	assert(transform);
	math3d_vec3_set(transform->position, 0.0f, 0.0f, 0.0f);
	math3d_vec3_set(transform->rotation, 0.0f, 0.0f, 0.0f);
	math3d_vec3_set(transform->scale, 1.0f, 1.0f, 1.0f);
}

void scene_init(scene_t* scene) {
	assert(scene);
	SDL_zero(*scene);
	scene->sun = UINT32_MAX;
}

entity_t scene_create(scene_t* scene, const char* name) {
	assert(scene);
	assert(name);
	for (entity_t entity = 0; entity < SCENE_MAX_ENTITIES; entity++) {
		if (scene->alive[entity]) {
			continue;
		}
		scene->alive[entity] = true;
		SDL_strlcpy(scene->names[entity], name, sizeof(scene->names[entity]));
		scene->count++;
		return entity;
	}
	SDL_Log("Scene capacity reached");
	return UINT32_MAX;
}

transform_component_t* scene_add_transform(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < SCENE_MAX_ENTITIES);
	scene->has_transform[entity] = true;
	set_default_transform(&scene->transforms[entity]);
	return &scene->transforms[entity];
}

renderable_component_t* scene_add_renderable(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < SCENE_MAX_ENTITIES);
	scene->has_renderable[entity] = true;
	renderable_component_t* renderable = &scene->renderables[entity];
	renderable->mesh = MESH_BOX;
	math3d_vec3_set(renderable->color, 0.8f, 0.8f, 0.8f);
	renderable->cast_shadow = true;
	return renderable;
}

directional_light_component_t* scene_add_directional_light(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < SCENE_MAX_ENTITIES);
	scene->has_light[entity] = true;
	scene->sun = entity;
	directional_light_component_t* light = &scene->lights[entity];
	math3d_vec3_set(light->direction, -0.45f, -1.0f, -0.25f);
	math3d_vec3_normalize(light->direction);
	math3d_vec3_set(light->color, 1.0f, 0.95f, 0.82f);
	light->intensity = 1.2f;
	return light;
}

oscillator_component_t* scene_add_oscillator(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < SCENE_MAX_ENTITIES);
	scene->has_oscillator[entity] = true;
	oscillator_component_t* oscillator = &scene->oscillators[entity];
	math3d_vec3_set(oscillator->origin, 0.0f, 0.0f, 0.0f);
	math3d_vec3_set(oscillator->axis, 0.0f, 1.0f, 0.0f);
	oscillator->amplitude = 1.0f;
	oscillator->speed = 1.0f;
	oscillator->phase = 0.0f;
	return oscillator;
}

spinner_component_t* scene_add_spinner(scene_t* scene, entity_t entity) {
	assert(scene);
	assert(entity < SCENE_MAX_ENTITIES);
	scene->has_spinner[entity] = true;
	spinner_component_t* spinner = &scene->spinners[entity];
	spinner->degrees_per_second = 35.0f;
	return spinner;
}

const directional_light_component_t* scene_main_light(const scene_t* scene) {
	assert(scene);
	if (scene->sun >= SCENE_MAX_ENTITIES || !scene->has_light[scene->sun]) {
		return NULL;
	}
	return &scene->lights[scene->sun];
}

void scene_update(scene_t* scene, float seconds) {
	assert(scene);
	for (entity_t entity = 0; entity < SCENE_MAX_ENTITIES; entity++) {
		if (!scene->alive[entity] || !scene->has_transform[entity]) {
			continue;
		}
		transform_component_t* transform = &scene->transforms[entity];
		if (scene->has_spinner[entity]) {
			const spinner_component_t* spinner = &scene->spinners[entity];
			transform->rotation[1] += rad(spinner->degrees_per_second) * seconds;
		}
		if (scene->has_oscillator[entity]) {
			const oscillator_component_t* oscillator = &scene->oscillators[entity];
			const float wave = sinf(SDL_GetTicks() * 0.001f * oscillator->speed + oscillator->phase);
			for (int i = 0; i < 3; i++) {
				transform->position[i] =
					oscillator->origin[i] + oscillator->axis[i] * oscillator->amplitude * wave;
			}
		}
	}
}

static entity_t add_box(scene_t* scene, const char* name, const float position[3], const float scale[3],
						const float color[3]) {
	entity_t entity = scene_create(scene, name);
	transform_component_t* transform = scene_add_transform(scene, entity);
	renderable_component_t* renderable = scene_add_renderable(scene, entity);
	SDL_memcpy(transform->position, position, sizeof(transform->position));
	SDL_memcpy(transform->scale, scale, sizeof(transform->scale));
	SDL_memcpy(renderable->color, color, sizeof(renderable->color));
	return entity;
}

void scene_build_default(scene_t* scene) {
	assert(scene);
	scene_init(scene);

	const float slate[] = {0.16f, 0.19f, 0.24f};
	const float sand[] = {0.72f, 0.69f, 0.58f};
	const float moss[] = {0.39f, 0.56f, 0.42f};
	const float coral[] = {0.82f, 0.47f, 0.36f};
	const float sky[] = {0.60f, 0.70f, 0.84f};

	entity_t sun = scene_create(scene, "sun");
	scene_add_directional_light(scene, sun);

	add_box(scene, "floor", (float[]){0.0f, -0.5f, 0.0f}, (float[]){60.0f, 1.0f, 60.0f}, sand);
	add_box(scene, "back_wall", (float[]){0.0f, 4.0f, -60.0f}, (float[]){60.0f, 9.0f, 1.0f}, slate);
	add_box(scene, "front_wall", (float[]){0.0f, 4.0f, 60.0f}, (float[]){60.0f, 9.0f, 1.0f}, slate);
	add_box(scene, "left_wall", (float[]){-60.0f, 4.0f, 0.0f}, (float[]){1.0f, 9.0f, 60.0f}, slate);
	add_box(scene, "right_wall", (float[]){60.0f, 4.0f, 0.0f}, (float[]){1.0f, 9.0f, 60.0f}, slate);

	add_box(scene, "platform_a", (float[]){-20.0f, 1.4f, -12.0f}, (float[]){8.0f, 0.8f, 8.0f}, moss);
	add_box(scene, "platform_b", (float[]){18.0f, 2.0f, 15.0f}, (float[]){7.0f, 0.8f, 12.0f}, sky);

	entity_t moving = add_box(scene, "moving_block", (float[]){0.0f, 3.0f, -26.0f},
							  (float[]){5.0f, 0.5f, 5.0f}, coral);
	oscillator_component_t* oscillator = scene_add_oscillator(scene, moving);
	math3d_vec3_set(oscillator->origin, 0.0f, 3.0f, -26.0f);
	math3d_vec3_set(oscillator->axis, 1.0f, 0.0f, 0.0f);
	oscillator->amplitude = 18.0f;
	oscillator->speed = 0.8f;
	oscillator->phase = rad(45.0f);

	entity_t monolith = add_box(scene, "monolith", (float[]){0.0f, 5.0f, 0.0f},
								(float[]){4.0f, 10.0f, 4.0f}, slate);
	scene_add_spinner(scene, monolith)->degrees_per_second = 18.0f;

	entity_t beacon = add_box(scene, "beacon", (float[]){24.0f, 6.5f, -24.0f},
							  (float[]){2.0f, 13.0f, 2.0f}, sky);
	scene_add_spinner(scene, beacon)->degrees_per_second = -26.0f;

	add_box(scene, "cover_1", (float[]){-24.0f, 2.5f, 20.0f}, (float[]){5.0f, 5.0f, 5.0f}, moss);
	add_box(scene, "cover_2", (float[]){-8.0f, 1.8f, 30.0f}, (float[]){3.0f, 3.0f, 10.0f}, coral);
	add_box(scene, "cover_3", (float[]){28.0f, 2.2f, 22.0f}, (float[]){8.0f, 4.0f, 4.0f}, sky);
}
