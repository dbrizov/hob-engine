#include "engine.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

#include "debug.h"
#include "logging.h"
#include "timer.h"
#include "engine/components/camera_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"

namespace hob {
    EngineConfig::EngineConfig(const std::filesystem::path& json_path) {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            debug::log_error("Cannot open engine config file: {}", json_path.string());
            return;
        }

        nlohmann::json json = nlohmann::json::parse(file);

        if (json.contains("graphics")) {
            const auto& g = json["graphics"];
            graphics_config.window_title =
                g.value("window_title", graphics_config.window_title);
            graphics_config.window_width =
                g.value("window_width", graphics_config.window_width);
            graphics_config.window_height =
                g.value("window_height", graphics_config.window_height);
            graphics_config.logical_resolution_width =
                g.value("logical_resolution_width", graphics_config.logical_resolution_width);
            graphics_config.logical_resolution_height =
                g.value("logical_resolution_height", graphics_config.logical_resolution_height);
            graphics_config.pixels_per_meter =
                g.value("pixels_per_meter", graphics_config.pixels_per_meter);
            graphics_config.target_fps =
                g.value("target_fps", graphics_config.target_fps);
            graphics_config.vsync_enabled =
                g.value("vsync_enabled", graphics_config.vsync_enabled);
        }

        if (json.contains("physics")) {
            const auto& p = json["physics"];
            if (p.contains("gravity")) {
                const auto& gravity = p["gravity"];
                physics_config.gravity.x = gravity.value("x", physics_config.gravity.x);
                physics_config.gravity.y = gravity.value("y", physics_config.gravity.y);
            }

            physics_config.ticks_per_second =
                p.value("ticks_per_second", physics_config.ticks_per_second);
            physics_config.sub_steps_per_tick =
                p.value("sub_steps_per_tick", physics_config.sub_steps_per_tick);
            physics_config.interpolation_enabled =
                p.value("interpolation_enabled", physics_config.interpolation_enabled);
        }
    }

    Engine::Engine(const EngineConfig& config)
        : m_sdl_context(config.graphics_config)
        , m_imgui_system(m_sdl_context)
        , m_console()
        , m_renderer(config, m_sdl_context, m_console)
        , m_timer(config)
        , m_input(m_sdl_context, m_renderer)
        , m_physics(config, m_console)
        , m_cursor(m_sdl_context, m_renderer, m_input)
        , m_entity_spawner(*this)
        , m_lua_script_system(*this) {
    }

    Engine::~Engine() {
        // Tear down entities (and their components) while every subsystem is still alive.
        // Avoids dangling references during member destruction.
        // In particular - LuaScriptComponent's sol::table must release its Lua registry slot before
        // LuaScriptSystem destroys the lua_State.
        m_entity_spawner.clear();
    }

    void Engine::run() {
        bool is_running = true;
        std::vector<Entity*> entities;
        std::vector<Entity*> ticking_entities;
        std::vector<Entity*> physics_entities;
        std::vector<const Entity*> renderable_entities;

        while (is_running) {
            m_timer.frame_start();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                m_imgui_system.process_event(event);

                if (event.type == SDL_EVENT_QUIT) {
                    is_running = false;
                }
                else if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.key == SDLK_GRAVE) {
                        if (!m_console.is_open()) {
                            m_is_os_cursor_visible_before_console_opened = m_cursor.is_os_cursor_visible();
                            m_cursor.set_os_cursor_visible(true);
                        }
                        else {
                            m_cursor.set_os_cursor_visible(m_is_os_cursor_visible_before_console_opened);
                        }

                        m_console.toggle_open();
                    }
                }
            }

            m_imgui_system.frame_start();

            m_entity_spawner.resolve_requests();
            m_entity_spawner.get_entities(entities);
            m_entity_spawner.get_ticking_entities(ticking_entities);
            m_entity_spawner.get_physics_entities(physics_entities);
            m_entity_spawner.get_renderable_entities(renderable_entities);

            const float delta_time = m_timer.get_delta_time();
            const float scaled_delta_time = delta_time * m_timer.get_time_scale();

            if (!m_console.is_open()) {
                m_input.tick(scaled_delta_time);
            }

            for (Entity* entity : ticking_entities) {
                entity->tick(scaled_delta_time);
            }

            m_physics.tick_entities(scaled_delta_time, physics_entities);

#ifndef NDEBUG
            for (Entity* entity : entities) {
                entity->debug_draw_tick(scaled_delta_time);
            }
#endif

            m_renderer.frame_start();

            render_entities(renderable_entities);
            render_debug_draws();
            m_cursor.render();

            m_renderer.frame_end();

            if (m_console.is_open()) {
                m_console.render();
            }

            m_imgui_system.frame_end();

            m_sdl_context.swap();

            m_timer.frame_end();
        }
    }

    bool Engine::is_initialized() const {
        return m_sdl_context.is_initialized() &&
               m_renderer.is_initialized() &&
               m_imgui_system.is_initialized();
    }

    SdlContext& Engine::get_sdl_context() {
        return m_sdl_context;
    }

    Console& Engine::get_console() {
        return m_console;
    }

    Renderer& Engine::get_renderer() {
        return m_renderer;
    }

    Timer& Engine::get_timer() {
        return m_timer;
    }

    Input& Engine::get_input() {
        return m_input;
    }

    Physics& Engine::get_physics() {
        return m_physics;
    }

    Cursor& Engine::get_cursor() {
        return m_cursor;
    }

    EntitySpawner& Engine::get_entity_spawner() {
        return m_entity_spawner;
    }

    LuaScriptSystem& Engine::get_lua_script_system() {
        return m_lua_script_system;
    }

    void Engine::render_entities(std::vector<const Entity*>& entities) {
        Entity* camera_entity = m_entity_spawner.get_camera_entity();
        CameraComponent* camera_component = camera_entity->get_component<CameraComponent>();
        TransformComponent* camera_transform = camera_entity->get_transform();
        Vector2 camera_position = camera_transform->get_position();

        std::sort(entities.begin(), entities.end(),
                  [](const Entity* a, const Entity* b) {
                      const SpriteComponent* a_sprite = a->get_component<SpriteComponent>();
                      const SpriteComponent* b_sprite = b->get_component<SpriteComponent>();

                      return a_sprite->get_z_index() < b_sprite->get_z_index();
                  });

        for (const Entity* entity : entities) {
            const TransformComponent* transform_comp = entity->get_transform();
            const SpriteComponent* sprite_comp = entity->get_component<SpriteComponent>();

            const TextureRef& texture = sprite_comp->get_texture();
            if (!texture.is_valid()) {
                continue;
            }

            const Matrix2x3 matrix = Matrix2x3::lerp(transform_comp->get_prev_local_matrix(),
                                                     transform_comp->get_local_matrix(),
                                                     m_physics.get_interpolation_fraction());

            const Vector2 tr_scale = transform_comp->get_scale();
            const Vector2 sp_scale = sprite_comp->get_scale();
            const Vector2 scale = Vector2(tr_scale.x * sp_scale.x, tr_scale.y * sp_scale.y);

            const float texture_width = static_cast<float>(texture.get_width());
            const float texture_height = static_cast<float>(texture.get_height());
            const float runtime_ppm = m_renderer.get_pixels_per_meter_f();
            const float sprite_ppm = sprite_comp->get_pixels_per_meter_f();
            const Vector2 size_pixels = Vector2((texture_width / sprite_ppm) * runtime_ppm * scale.x,
                                                (texture_height / sprite_ppm) * runtime_ppm * scale.y);

            const Vector2 sprite_pivot = sprite_comp->get_pivot();
            const Vector2 world_position = matrix.origin;
            Vector2 screen_pos = camera_component->world_to_screen(world_position, camera_position);
            screen_pos.x -= size_pixels.x * sprite_pivot.x;
            screen_pos.y -= size_pixels.y * sprite_pivot.y;

            const Vector2 pivot_pixel(size_pixels.x * sprite_pivot.x, size_pixels.y * sprite_pivot.y);

            m_renderer.draw_sprite(
                texture.get_id(),
                screen_pos,
                size_pixels,
                pivot_pixel,
                matrix.get_rotation(),
                sprite_comp->get_tint());
        }
    }

    void Engine::render_debug_draws() {
        Entity* camera_entity = m_entity_spawner.get_camera_entity();
        CameraComponent* camera_component = camera_entity->get_component<CameraComponent>();

        debug::render_debug_draws(m_renderer, camera_component);
    }
}
