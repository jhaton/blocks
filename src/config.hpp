#pragma once

#include <cstddef>
#include <string_view>

namespace starter::config {

inline constexpr std::string_view kAppName = "sdl_gpu_fps_starter";
inline constexpr int kWindowWidth = 1600;
inline constexpr int kWindowHeight = 900;
inline constexpr bool kDeviceValidation = false;

inline constexpr float kPlayerHeight = 1.8f;
inline constexpr float kPlayerMoveSpeed = 6.0f;
inline constexpr float kPlayerFastMultiplier = 2.5f;
inline constexpr float kPlayerSlowMultiplier = 0.35f;
inline constexpr float kPlayerMouseSensitivity = 0.10f;
inline constexpr float kPlayerGravity = 28.0f;
inline constexpr float kPlayerGroundY = 0.0f;
inline constexpr float kPlayerResetY = -40.0f;

inline constexpr std::size_t kShadowMapSize = 2048;
inline constexpr float kShadowDistance = 28.0f;
inline constexpr float kShadowCameraHeight = 18.0f;

inline constexpr std::size_t kSceneMaxEntities = 128;
inline constexpr std::string_view kSavePath = "starter.save";

} // namespace starter::config
