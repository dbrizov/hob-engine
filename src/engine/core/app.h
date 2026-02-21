#ifndef HOB_ENGINE_APP_H
#define HOB_ENGINE_APP_H
#include "app_console.h"
#include "assets.h"
#include "imgui_system.h"
#include "input.h"
#include "physics.h"
#include "sdl_context.h"
#include "timer.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    struct GraphicsConfig {
        std::string window_title = "SDL2 Window";
        uint32_t window_width = 1152;
        uint32_t window_height = 648;
        uint32_t logical_resolution_width = 1152;
        uint32_t logical_resolution_height = 648;
        uint32_t pixels_per_meter = 64;
        uint32_t target_fps = 60;
        bool vsync_enabled = true;
    };

    struct PhysicsConfig {
        Vector2 gravity = Vector2(0.0f, -9.81f);
        uint32_t ticks_per_second = 60;
        uint32_t sub_steps_per_tick = 4;
        bool interpolation_enabled = true;
    };

    struct AppConfig {
        GraphicsConfig graphics_config;
        PhysicsConfig physics_config;
    };

    class App {
        AppConfig m_config;

        // SDL context must be declared first so it is destroyed last.
        // Objects below depend on SDL resources.
        SdlContext m_sdl_context;
        ImGuiSystem m_imgui_system;
        AppConsole m_app_console;
        Timer m_timer;
        Input m_input;
        Assets m_assets;
        Physics m_physics;
        EntitySpawner m_entity_spawner;

    public:
        explicit App(const AppConfig& config);

        void run();

        bool is_initialized() const;

        const AppConfig& get_config() const;

        Timer& get_timer();
        Input& get_input();
        Assets& get_assets();
        Physics& get_physics();
        EntitySpawner& get_entity_spawner();

    private:
        void render_entities(const std::vector<const Entity*>& entities);
    };
}

#endif //HOB_ENGINE_APP_H
