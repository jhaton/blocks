#include "config.hpp"
#include "frame_input.hpp"
#include "game.hpp"
#include "helpers.hpp"
#include "renderer.hpp"
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdlib>

namespace starter {

struct app_t {
	SDL_Window* window;
	renderer_t renderer;
	Game game;
	bool running;
	bool relative_mouse;
};

static void app_toggle_mouse_capture(app_t* app, bool enabled) {
	assert(app);
	app->relative_mouse = enabled;
	SDL_SetWindowRelativeMouseMode(app->window, enabled);
}

static FrameInput app_poll(app_t* app) {
	assert(app);
	FrameInput input;
	const bool* keys = SDL_GetKeyboardState(NULL);
	input.move_forward = keys[SDL_SCANCODE_W];
	input.move_backward = keys[SDL_SCANCODE_S];
	input.move_left = keys[SDL_SCANCODE_A];
	input.move_right = keys[SDL_SCANCODE_D];
	input.move_fast = keys[SDL_SCANCODE_LSHIFT];
	input.move_slow = keys[SDL_SCANCODE_LCTRL];

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			input.quit_requested = true;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (!app->relative_mouse) {
				input.capture_mouse_requested = true;
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if (app->relative_mouse) {
				input.mouse_delta_x += event.motion.xrel;
				input.mouse_delta_y += event.motion.yrel;
			}
			break;
		case SDL_EVENT_KEY_DOWN:
			switch (event.key.scancode) {
			case SDL_SCANCODE_ESCAPE:
				input.release_mouse_requested = true;
				break;
			case SDL_SCANCODE_F11:
				input.toggle_fullscreen_requested = true;
				break;
			case SDL_SCANCODE_F5:
				input.save_requested = true;
				break;
			case SDL_SCANCODE_F1:
				input.toggle_debug_requested = true;
				break;
			case SDL_SCANCODE_F2:
				input.toggle_debug_panel_requested = true;
				break;
			case SDL_SCANCODE_F3:
				input.toggle_debug_colliders_requested = true;
				break;
			case SDL_SCANCODE_F4:
				input.toggle_debug_player_requested = true;
				break;
			case SDL_SCANCODE_F6:
				input.toggle_debug_axes_requested = true;
				break;
			case SDL_SCANCODE_F7:
				input.toggle_debug_shadow_requested = true;
				break;
			case SDL_SCANCODE_F9:
				input.load_requested = true;
				break;
			case SDL_SCANCODE_SPACE:
				input.jump_requested = true;
				break;
			default:
				break;
			}
			break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			input.pixel_size_changed = true;
			input.pixel_width = event.window.data1;
			input.pixel_height = event.window.data2;
			break;
		default:
			break;
		}
	}
	return input;
}

static bool app_init(app_t* app) {
	assert(app);
	SDL_zero(*app);
	SDL_SetAppMetadata(config::kAppName.data(), "0.1.0", "com.example.sdl_gpu_fps_starter");
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return false;
	}

	app->window = SDL_CreateWindow(config::kAppName.data(), config::kWindowWidth, config::kWindowHeight,
								   SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (!check_resource(app->window, "create window")) {
		return false;
	}

	if (!renderer_init(&app->renderer, app->window, config::kDeviceValidation)) {
		return false;
	}

	if (!app->game.init()) {
		return false;
	}

	int width = 0;
	int height = 0;
	SDL_GetWindowSizeInPixels(app->window, &width, &height);
	camera_set_viewport(&app->game.player_camera, width, height);
	app->running = true;
	return true;
}

static void app_shutdown(app_t* app) {
	assert(app);
	if (app->window) {
		app->game.request_save();
	}
	renderer_destroy(&app->renderer);
	if (app->window) {
		SDL_DestroyWindow(app->window);
	}
	SDL_Quit();
}

static void app_apply_window_actions(app_t* app, const FrameInput& input) {
	if (input.capture_mouse_requested) {
		app_toggle_mouse_capture(app, true);
	}
	if (input.release_mouse_requested) {
		app_toggle_mouse_capture(app, false);
	}
	if (input.toggle_fullscreen_requested) {
		if (SDL_GetWindowFlags(app->window) & SDL_WINDOW_FULLSCREEN) {
			SDL_SetWindowFullscreen(app->window, false);
			app_toggle_mouse_capture(app, false);
		} else {
			SDL_SetWindowFullscreen(app->window, true);
			app_toggle_mouse_capture(app, true);
		}
	}
	if (input.pixel_size_changed) {
		camera_set_viewport(&app->game.player_camera, input.pixel_width, input.pixel_height);
	}
}

} // namespace starter

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	starter::app_t app;
	if (!starter::app_init(&app)) {
		starter::app_shutdown(&app);
		return EXIT_FAILURE;
	}

	Uint64 previous_counter = SDL_GetPerformanceCounter();
	while (app.running) {
		Uint64 current_counter = SDL_GetPerformanceCounter();
		const float frequency = (float)SDL_GetPerformanceFrequency();
		const float seconds = (current_counter - previous_counter) / frequency;
		previous_counter = current_counter;
		starter::FrameInput input = starter::app_poll(&app);
		if (input.quit_requested) {
			app.running = false;
		}
		starter::app_apply_window_actions(&app, input);
		if (!app.relative_mouse) {
			input.mouse_delta_x = 0.0f;
			input.mouse_delta_y = 0.0f;
			input.move_forward = false;
			input.move_backward = false;
			input.move_left = false;
			input.move_right = false;
			input.move_fast = false;
			input.move_slow = false;
			input.jump_requested = false;
		}
		app.game.update(input, seconds);
		renderer_draw(&app.renderer, &app.game.scene, &app.game.player_camera, app.game.elapsed_seconds,
					  &app.game.debug);
	}

	starter::app_shutdown(&app);
	return EXIT_SUCCESS;
}
