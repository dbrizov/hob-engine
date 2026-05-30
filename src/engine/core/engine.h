#pragma once

#include <filesystem>

#include "console.h"
#include "cursor.h"
#include "imgui_system.h"
#include "input.h"
#include "lua_script_system.h"
#include "physics.h"
#include "renderer.h"
#include "sdl_context.h"
#include "timer.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    struct GraphicsConfig {
        std::string window_title = "Hob Engine";
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

    struct EngineConfig {
        GraphicsConfig graphics_config;
        PhysicsConfig physics_config;

        EngineConfig() = default;
        explicit EngineConfig(const std::filesystem::path& json_path);
    };

    class Engine {
        // Order matters
        SdlContext m_sdl_context;
        ImGuiSystem m_imgui_system;
        Console m_console;
        Renderer m_renderer;
        Timer m_timer;
        Input m_input;
        Physics m_physics;
        Cursor m_cursor;
        EntitySpawner m_entity_spawner;
        LuaScriptSystem m_lua_script_system;

        bool m_is_os_cursor_visible_before_console_opened = false;

    public:
        explicit Engine(const EngineConfig& config);
        ~Engine();

        void run();

        bool is_initialized() const;

        SdlContext& get_sdl_context();
        Console& get_console();
        Renderer& get_renderer();
        Timer& get_timer();
        Input& get_input();
        Physics& get_physics();
        Cursor& get_cursor();
        EntitySpawner& get_entity_spawner();
        LuaScriptSystem& get_lua_script_system();

    private:
        void render_entities(std::vector<const Entity*>& entities);
        void render_debug_draws();
    };
}
