#pragma once

#include "engine/entity/entity_spawner.h"
#include "systems/console.h"
#include "systems/cursor.h"
#include "systems/imgui_system.h"
#include "systems/input.h"
#include "systems/physics.h"
#include "systems/renderer.h"
#include "systems/scripting/lua_script_system.h"
#include "systems/sdl_context.h"
#include "systems/timer.h"

namespace hob {
    struct EngineConfig;

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
        void draw_entities(std::vector<const Entity*>& entities);
        void flush_debug_draws_to_renderer(float delta_time);
    };
}
