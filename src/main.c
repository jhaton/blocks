#include "camera.h"
#include "config.h"
#include "helpers.h"
#include "renderer.h"
#include "save.h"
#include "scene.h"
#include "system_player.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
	SDL_Window* window;
	renderer_t renderer;
	scene_t scene;
	camera_t player_camera;
	bool running;
	bool relative_mouse;
	float elapsed_seconds;
} app_t;

static void app_toggle_mouse_capture(app_t* app, bool enabled) {
	assert(app);
	app->relative_mouse = enabled;
	SDL_SetWindowRelativeMouseMode(app->window, enabled);
}

static bool app_init(app_t* app) {
	assert(app);
	SDL_zero(*app);
	SDL_SetAppMetadata(APP_NAME, "0.1.0", "com.example.sdl_gpu_fps_starter");
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return false;
	}

	app->window = SDL_CreateWindow(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT,
								   SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (!check_resource(app->window, "create window")) {
		return false;
	}

	if (!renderer_init(&app->renderer, app->window, DEVICE_VALIDATION)) {
		return false;
	}

	scene_build_default(&app->scene);
	camera_init(&app->player_camera, CAMERA_TYPE_PERSPECTIVE);
	system_player_apply_spawn(&app->scene, scene_player(&app->scene), &app->player_camera);
	if (save_load_player(SAVE_PATH, &app->player_camera)) {
		system_player_capture_camera(&app->scene, scene_player(&app->scene), &app->player_camera);
	}
	system_player_sync_camera(&app->scene, scene_player(&app->scene), &app->player_camera);

	int width = 0;
	int height = 0;
	SDL_GetWindowSizeInPixels(app->window, &width, &height);
	camera_set_viewport(&app->player_camera, width, height);
	app->running = true;
	return true;
}

static void app_shutdown(app_t* app) {
	assert(app);
	if (app->window) {
		save_write_player(SAVE_PATH, &app->player_camera);
	}
	renderer_destroy(&app->renderer);
	if (app->window) {
		SDL_DestroyWindow(app->window);
	}
	SDL_Quit();
}

static void app_handle_keydown(app_t* app, const SDL_KeyboardEvent* key) {
	assert(app);
	assert(key);
	switch (key->scancode) {
	case SDL_SCANCODE_ESCAPE:
		app_toggle_mouse_capture(app, false);
		break;
	case SDL_SCANCODE_F11:
		if (SDL_GetWindowFlags(app->window) & SDL_WINDOW_FULLSCREEN) {
			SDL_SetWindowFullscreen(app->window, false);
			app_toggle_mouse_capture(app, false);
		} else {
			SDL_SetWindowFullscreen(app->window, true);
			app_toggle_mouse_capture(app, true);
		}
		break;
	case SDL_SCANCODE_F5:
		save_write_player(SAVE_PATH, &app->player_camera);
		break;
	case SDL_SCANCODE_F9:
		if (!save_load_player(SAVE_PATH, &app->player_camera)) {
			system_player_apply_spawn(&app->scene, scene_player(&app->scene), &app->player_camera);
		} else {
			system_player_capture_camera(&app->scene, scene_player(&app->scene), &app->player_camera);
		}
		system_player_sync_camera(&app->scene, scene_player(&app->scene), &app->player_camera);
		break;
	default:
		break;
	}
}

static void app_poll(app_t* app) {
	assert(app);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			app->running = false;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (!app->relative_mouse) {
				app_toggle_mouse_capture(app, true);
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (app->relative_mouse) {
				camera_rotate(&app->player_camera, -event.motion.yrel * PLAYER_MOUSE_SENSITIVITY,
							  event.motion.xrel * PLAYER_MOUSE_SENSITIVITY);
			}
			break;
		case SDL_EVENT_KEY_DOWN:
			app_handle_keydown(app, &event.key);
			break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			camera_set_viewport(&app->player_camera, event.window.data1, event.window.data2);
			break;
		}
	}
}

static void app_move_player(app_t* app, float seconds) {
	assert(app);
	if (!app->relative_mouse) {
		return;
	}
	system_player_move(&app->scene, scene_player(&app->scene), &app->player_camera,
					   SDL_GetKeyboardState(NULL), seconds);
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	app_t app;
	if (!app_init(&app)) {
		app_shutdown(&app);
		return EXIT_FAILURE;
	}

	Uint64 previous_counter = SDL_GetPerformanceCounter();
	while (app.running) {
		Uint64 current_counter = SDL_GetPerformanceCounter();
		const float frequency = (float)SDL_GetPerformanceFrequency();
		const float seconds = (current_counter - previous_counter) / frequency;
		previous_counter = current_counter;
		app.elapsed_seconds += seconds;

		app_poll(&app);
		app_move_player(&app, seconds);
		system_player_apply_gravity(&app.scene, scene_player(&app.scene), seconds);
		system_player_respawn(&app.scene, scene_player(&app.scene), &app.player_camera);
		system_player_sync_camera(&app.scene, scene_player(&app.scene), &app.player_camera);
		scene_update(&app.scene, seconds);
		camera_update(&app.player_camera);
		renderer_draw(&app.renderer, &app.scene, &app.player_camera, app.elapsed_seconds);
	}

	app_shutdown(&app);
	return EXIT_SUCCESS;
}
