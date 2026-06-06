#include "engine.h"
#include "engine_config.h"

#include "debug.h"
#include "engine/components/camera_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/math/matrix2x3.h"

namespace hob {
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

            m_imgui_system.new_frame();

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

            // Draw
            draw_entities(renderable_entities);
            flush_debug_draws_to_renderer(scaled_delta_time);
            m_cursor.draw();

            if (m_console.is_open()) {
                m_console.draw();
            }

            // Render
            m_renderer.set_frame_time(m_timer.get_play_time());
            if (m_renderer.acquire_command_buffer()) {
                m_renderer.render_world_pass();
                m_renderer.render_blit_pass();
                m_renderer.render_debug_lines_pass();
                m_renderer.render_debug_text_pass();
                m_imgui_system.render_pass(m_renderer.get_command_buffer(), m_renderer.get_swap_texture());
                m_renderer.render_overlay_pass();

                m_renderer.submit_command_buffer();
            }
            else {
                m_imgui_system.discard_frame();
                m_renderer.cancel_command_buffer();
            }

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

    CameraComponent* Engine::get_active_camera() const {
        if (m_active_camera == nullptr) {
            debug::log_error(
                "Engine::get_active_camera: no active camera (spawn a Camera entity before any rendering or camera query)");
        }
        return m_active_camera;
    }

    void Engine::set_active_camera(CameraComponent* camera) {
        m_active_camera = camera;
    }

    void Engine::clear_active_camera(CameraComponent* camera) {
        if (m_active_camera == camera) {
            m_active_camera = nullptr;
        }
    }

    void Engine::draw_entities(std::vector<const Entity*>& entities) {
        CameraComponent* camera = get_active_camera();
        if (camera == nullptr) {
            return;
        }

        const float camera_ppm = camera->get_screen_pixels_per_meter();
        TransformComponent* camera_transform = camera->get_entity().get_transform();
        const Vector2 camera_position = camera_transform->get_position();

        for (const Entity* entity : entities) {
            const TransformComponent* transform_comp = entity->get_transform();
            const SpriteComponent* sprite_comp = entity->get_component<SpriteComponent>();

            const TextureRef& texture = sprite_comp->get_texture();
            if (texture == nullptr) {
                continue;
            }

            const Matrix2x3 matrix = transform_comp->get_interpolate_physics()
                                         ? Matrix2x3::lerp(transform_comp->get_prev_local_matrix(),
                                                           transform_comp->get_local_matrix(),
                                                           m_physics.get_interpolation_fraction())
                                         : transform_comp->get_local_matrix();

            const Vector2 tr_scale = transform_comp->get_scale();
            const Vector2 sp_scale = sprite_comp->get_scale();
            const Vector2 scale = Vector2(tr_scale.x * sp_scale.x, tr_scale.y * sp_scale.y);

            const float texture_width = static_cast<float>(texture->get_width());
            const float texture_height = static_cast<float>(texture->get_height());
            const float sprite_ppm = sprite_comp->get_pixels_per_meter_f();
            const Vector2 size_pixels = Vector2((texture_width / sprite_ppm) * camera_ppm * scale.x,
                                                (texture_height / sprite_ppm) * camera_ppm * scale.y);

            const Vector2 sprite_pivot = sprite_comp->get_pivot();
            const Vector2 world_position = matrix.origin;
            Vector2 screen_pos = camera->world_to_screen(world_position, camera_position);
            screen_pos.x -= size_pixels.x * sprite_pivot.x;
            screen_pos.y -= size_pixels.y * sprite_pivot.y;

            const Vector2 pivot_pixel(size_pixels.x * sprite_pivot.x, size_pixels.y * sprite_pivot.y);

            m_renderer.draw_sprite(
                texture,
                screen_pos,
                size_pixels,
                pivot_pixel,
                matrix.get_rotation(),
                sprite_comp->get_z_index(),
                sprite_comp->get_material());
        }
    }

    void Engine::flush_debug_draws_to_renderer(float delta_time) {
        CameraComponent* camera = get_active_camera();
        if (camera == nullptr) {
            return;
        }

        debug::flush_draws_to_renderer(m_renderer,
                                       camera,
                                       m_sdl_context.get_window_size(),
                                       delta_time);
    }
}
