# SDL_GPU FPS Starter

Tiny first-person starter repo in pragmatic C++ using SDL3's GPU API, CMake, `just`, and shader cross-compilation for Vulkan and Metal.

It is intentionally small on day one, but structured so adding render passes, gameplay systems, entities, and persistence is obvious. The codebase aims for a "C with C++ skin" style: plain data, explicit update order, minimal abstraction, and a little more type safety and ergonomics than straight C.

## What This Repo Keeps

- SDL3 GPU bootstrap and swapchain setup
- CMake build with shader compilation
- `just` recipes for configure/build/run
- Vulkan SPIR-V shaders with automatic Metal translation on macOS
- A compact first-person camera and input loop
- A small homemade ECS-style scene layer
- A thin save/load service using a tiny text format

## Default Scene

The starter launches into a stylized graybox arena:

- a walkable floor and enclosing walls
- a directional light with a shadow map
- a spinning monolith
- a moving platform driven by a behavior component
- gravity and fall-reset for the player
- a switch that opens a gate
- a built-in debug overlay and world debug draw toggles

The scene is box-based on purpose so the rendering and entity structure stay easy to read.

## Architecture

### Render passes

The frame is organized as explicit passes in [`renderer.cpp`](/Users/jason/github/jhaton/blocks/src/renderer.cpp):

1. `shadow`
2. `scene`
3. `debug`
4. `overlay`

Each pass lives in its own file:

- [`render_pass_shadow.cpp`](/Users/jason/github/jhaton/blocks/src/render_pass_shadow.cpp)
- [`render_pass_scene.cpp`](/Users/jason/github/jhaton/blocks/src/render_pass_scene.cpp)
- [`render_pass_debug.cpp`](/Users/jason/github/jhaton/blocks/src/render_pass_debug.cpp)
- [`render_pass_overlay.cpp`](/Users/jason/github/jhaton/blocks/src/render_pass_overlay.cpp)

To add another pass, copy the same pattern:

- add shaders
- create a new `render_pass_*.cpp`
- register it in the renderer pass array
- give it any textures/samplers it needs

### Entities and behavior

The scene layer is in [`scene.hpp`](/Users/jason/github/jhaton/blocks/src/scene.hpp) and [`scene.cpp`](/Users/jason/github/jhaton/blocks/src/scene.cpp). Runtime orchestration lives in [`game.hpp`](/Users/jason/github/jhaton/blocks/src/game.hpp) and [`game.cpp`](/Users/jason/github/jhaton/blocks/src/game.cpp).

It uses a small homemade ECS-like layout:

- entity IDs
- parallel component arrays
- systems that iterate matching components
- a `Game` layer that owns system order and frame flow

Included starter components:

- `Transform`
- `Renderable`
- `DirectionalLight`
- `Oscillator`
- `Spinner`
- `PlayerController`
- `CharacterBody`
- `Gravity`
- `Respawn`
- `BoxCollider`

Included starter systems:

- [`system_door.cpp`](/Users/jason/github/jhaton/blocks/src/system_door.cpp)
- [`system_interaction.cpp`](/Users/jason/github/jhaton/blocks/src/system_interaction.cpp)
- [`system_physics.cpp`](/Users/jason/github/jhaton/blocks/src/system_physics.cpp)
- [`system_player.cpp`](/Users/jason/github/jhaton/blocks/src/system_player.cpp)
- [`system_spinner.cpp`](/Users/jason/github/jhaton/blocks/src/system_spinner.cpp)
- [`system_oscillator.cpp`](/Users/jason/github/jhaton/blocks/src/system_oscillator.cpp)

Included scene recipes:

- [`scene_spawn_player`](/Users/jason/github/jhaton/blocks/src/scene.cpp)
- [`scene_spawn_static_solid`](/Users/jason/github/jhaton/blocks/src/scene.cpp)
- [`scene_spawn_interaction_switch`](/Users/jason/github/jhaton/blocks/src/scene.cpp)
- [`scene_spawn_sliding_door`](/Users/jason/github/jhaton/blocks/src/scene.cpp)
- [`scene_spawn_spinning_prop`](/Users/jason/github/jhaton/blocks/src/scene.cpp)
- [`scene_spawn_moving_platform`](/Users/jason/github/jhaton/blocks/src/scene.cpp)

To add behavior:

- add component data to [`scene.hpp`](/Users/jason/github/jhaton/blocks/src/scene.hpp)
- add setup helpers in [`scene.cpp`](/Users/jason/github/jhaton/blocks/src/scene.cpp) if needed
- create a new `system_*.cpp`
- call it from [`game.cpp`](/Users/jason/github/jhaton/blocks/src/game.cpp) in a clear order

The intended split is:

- `scene` owns data
- `system_*` files own behavior
- `game` owns orchestration
- `main` owns SDL app lifetime and window/input plumbing

### Persistence

The save service lives in [`save.cpp`](/Users/jason/github/jhaton/blocks/src/save.cpp).

Current file format:

```txt
version 1
player_position 0.000000 1.800000 9.500000
player_rotation -0.139626 3.141593
```

It is intentionally trivial to parse and easy to replace later with a richer format. The clean extension point is the `Game` layer: keep save/load policy in [`game.cpp`](/Users/jason/github/jhaton/blocks/src/game.cpp) and keep the file format logic in [`save.cpp`](/Users/jason/github/jhaton/blocks/src/save.cpp).

The default save now persists both player transform and the example interaction state so the starter shows how world state can survive reloads.

### Debugging

Runtime debug state lives in [`debug.hpp`](/Users/jason/github/jhaton/blocks/src/debug.hpp) and [`debug.cpp`](/Users/jason/github/jhaton/blocks/src/debug.cpp).

The template now ships with:

- an in-game text panel for frame, player, entity, and pass state
- world-space debug drawing for colliders, player body, transforms, and shadow camera direction
- hotkey toggles so you can inspect one concern at a time without changing code

## Controls

- `WASD` move on the ground plane
- `Space` jump
- `E` use focused interactable
- mouse look after clicking into the window
- `LShift` move faster
- `LCtrl` move slower
- `Esc` release mouse capture
- `F11` toggle fullscreen
- `F1` toggle debug
- `F2` toggle debug panel
- `F3` toggle collider draw
- `F4` toggle player body draw
- `F6` toggle transform axes
- `F7` toggle shadow debug draw
- `F5` save player state
- `F9` reload player state

## Build

### macOS

Install `glslc` and either `shadercross` or `spirv-cross`.

### Linux

Install `glslc`.

### Windows

Install the Vulkan SDK so `glslc` is available.

Then:

```bash
git clone https://github.com/jsoulier/blocks --recurse-submodules
cd blocks
just run-debug
```

Or use the CMake flow directly:

```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug --parallel
cd build/debug/bin
./sdl_gpu_fps_starter
```
