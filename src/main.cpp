#include "engine/core/app.h"
#include "engine/math/vector2.h"

const std::string WINDOW_TITLE = "Hob Engine";
constexpr uint32_t WINDOW_WIDTH = 1152;
constexpr uint32_t WINDOW_HEIGHT = 648;
constexpr uint32_t LOGICAL_RESOLUTION_WIDTH = 1920;
constexpr uint32_t LOGICAL_RESOLUTION_HEIGHT = 1080;
constexpr uint32_t PIXELS_PER_METER = 64;
constexpr uint32_t TARGET_FPS = 60;
constexpr bool VSYNC_ENABLED = true;

const hob::Vector2 PHYSICS_GRAVITY = hob::Vector2(0.0f, -9.81f);
constexpr uint32_t PHYSICS_TICKS_PER_SECOND = 60;
constexpr uint32_t PHYSICS_SUB_STEPS_PER_TICK = 4;
constexpr bool PHYSICS_INTERPOLATION_ENABLED = false;

int main(int argc, char* argv[]) {
    hob::GraphicsConfig graphics_config;
    graphics_config.window_title = WINDOW_TITLE;
    graphics_config.window_width = WINDOW_WIDTH;
    graphics_config.window_height = WINDOW_HEIGHT;
    graphics_config.logical_resolution_width = LOGICAL_RESOLUTION_WIDTH;
    graphics_config.logical_resolution_height = LOGICAL_RESOLUTION_HEIGHT;
    graphics_config.pixels_per_meter = PIXELS_PER_METER;
    graphics_config.target_fps = TARGET_FPS;
    graphics_config.vsync_enabled = VSYNC_ENABLED;

    hob::PhysicsConfig physics_config;
    physics_config.gravity = PHYSICS_GRAVITY;
    physics_config.ticks_per_second = PHYSICS_TICKS_PER_SECOND;
    physics_config.sub_steps_per_tick = PHYSICS_SUB_STEPS_PER_TICK;
    physics_config.interpolation_enabled = PHYSICS_INTERPOLATION_ENABLED;

    hob::AppConfig app_config;
    app_config.graphics_config = graphics_config;
    app_config.physics_config = physics_config;

    hob::App app(app_config);

    if (!app.is_initialized()) {
        return 1;
    }

    app.run();

    return 0;
}
