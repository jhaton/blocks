#pragma once

#include "camera.hpp"
#include "frame_input.hpp"
#include "scene.hpp"

void system_interaction_focus(scene_t* scene, const camera_t* camera);
void system_interaction_use(scene_t* scene, const starter::FrameInput& input);
