# SDL_GPU FPS Starter

Tiny first-person starter repo in C using SDL3's GPU API, CMake, `just`, and shader cross-compilation for Vulkan and Metal.

It is intentionally small on day one, but structured so adding render passes, gameplay systems, entities, and persistence is obvious.

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
- a simple overlay crosshair

The scene is box-based on purpose so the rendering and entity structure stay easy to read.

## Architecture

### Render passes

The frame is organized as explicit passes in [`src/renderer.c`](/Users/jason/github/jhaton/blocks/src/renderer.c):

1. `shadow`
2. `scene`
3. `overlay`

Each pass lives in its own file:

- [`src/render_pass_shadow.c`](/Users/jason/github/jhaton/blocks/src/render_pass_shadow.c)
- [`src/render_pass_scene.c`](/Users/jason/github/jhaton/blocks/src/render_pass_scene.c)
- [`src/render_pass_overlay.c`](/Users/jason/github/jhaton/blocks/src/render_pass_overlay.c)

To add another pass, copy the same pattern:

- add shaders
- create a new `render_pass_*.c`
- register it in the renderer pass array
- give it any textures/samplers it needs

### Entities and behavior

The scene layer is in [`src/scene.h`](/Users/jason/github/jhaton/blocks/src/scene.h) and [`src/scene.c`](/Users/jason/github/jhaton/blocks/src/scene.c).

It uses a small homemade ECS-like layout:

- entity IDs
- parallel component arrays
- update systems that iterate matching components

Included starter components:

- `Transform`
- `Renderable`
- `DirectionalLight`
- `Oscillator`
- `Spinner`

To add behavior, create a new component array plus a small update loop in `scene_update`.

### Persistence

The save service lives in [`src/save.c`](/Users/jason/github/jhaton/blocks/src/save.c).

Current file format:

```txt
version 1
player_position 0.000000 1.800000 9.500000
player_rotation -0.139626 3.141593
```

It is intentionally trivial to parse from either C or future lightweight C++ code.

## Controls

- `WASD` move on the ground plane
- `Q/E` move down/up
- mouse look after clicking into the window
- `LShift` move faster
- `LCtrl` move slower
- `Esc` release mouse capture
- `F11` toggle fullscreen
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
