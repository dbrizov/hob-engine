#include "app.h"

#include <SDL3/SDL.h>
#include <algorithm>

#include "debug.h"
#include "timer.h"
#include "engine/components/camera_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/math/constants.h"

namespace hob {
    App::App(const AppConfig& config)
        : m_config(config)
        , m_sdl_context(config.graphics_config)
        , m_renderer(m_sdl_context.get_window(), config.graphics_config)
        , m_imgui_system(m_sdl_context.get_window(), m_sdl_context.get_gl_context())
        , m_console()
        , m_timer(config.graphics_config.target_fps, config.graphics_config.vsync_enabled)
        , m_input(*this)
        , m_assets()
        , m_physics(*this)
        , m_entity_spawner(*this) {
    }

    void App::run() {
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

            m_renderer.frame_end();

            if (m_console.is_open()) {
                m_console.render();
            }

            m_imgui_system.frame_end();

            m_sdl_context.swap();

            m_timer.frame_end();
        }
    }

    bool App::is_initialized() const {
        return m_sdl_context.is_initialized() &&
               m_renderer.is_initialized() &&
               m_imgui_system.is_initialized();
    }

    const AppConfig& App::get_config() const {
        return m_config;
    }

    SdlContext& App::get_sdl_context() {
        return m_sdl_context;
    }

    Renderer& App::get_renderer() {
        return m_renderer;
    }

    Console& App::get_console() {
        return m_console;
    }

    Timer& App::get_timer() {
        return m_timer;
    }

    Input& App::get_input() {
        return m_input;
    }

    Assets& App::get_assets() {
        return m_assets;
    }

    Physics& App::get_physics() {
        return m_physics;
    }

    EntitySpawner& App::get_entity_spawner() {
        return m_entity_spawner;
    }

    void App::render_entities(std::vector<const Entity*>& entities) {
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

            int texture_width = 0;
            int texture_height = 0;
            m_assets.get_texture_size(sprite_comp->get_texture_id(), texture_width, texture_height);

            const Matrix2x3 matrix = Matrix2x3::lerp(transform_comp->get_prev_local_matrix(),
                                                     transform_comp->get_local_matrix(),
                                                     m_physics.get_interpolation_fraction());

            const Vector2 sprite_pivot = sprite_comp->get_pivot();

            const Vector2 transform_scale = transform_comp->get_scale();
            const Vector2 sprite_scale = sprite_comp->get_scale();
            const Vector2 scale = Vector2(transform_scale.x * sprite_scale.x, transform_scale.y * sprite_scale.y);

            const float f_texture_width = static_cast<float>(texture_width);
            const float f_texture_height = static_cast<float>(texture_height);
            const Vector2 world_position = matrix.origin;
            Vector2 screen_position = camera_component->world_to_screen(world_position, camera_position);
            screen_position.x -= f_texture_width * sprite_pivot.x * scale.x;
            screen_position.y -= f_texture_height * sprite_pivot.y * scale.y;

            const Vector2 size = Vector2(f_texture_width * scale.x, f_texture_height * scale.y);
            const Vector2 pivot_pixel(size.x * sprite_pivot.x, size.y * sprite_pivot.y);

            const float rotation_rad = matrix.get_rotation();

            m_renderer.draw_sprite(
                m_assets.get_texture(sprite_comp->get_texture_id()),
                screen_position,
                size,
                pivot_pixel,
                rotation_rad,
                sprite_comp->get_tint());
        }
    }

    void App::render_debug_draws() {
        Entity* camera_entity = m_entity_spawner.get_camera_entity();
        CameraComponent* camera_component = camera_entity->get_component<CameraComponent>();

        debug::render_debug_draws(m_renderer, camera_component);
    }
}
