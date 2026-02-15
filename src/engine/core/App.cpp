#include "App.h"

#include <SDL.h>

#include "Debug.h"
#include "Timer.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/TransformComponent.h"

App::App(const AppConfig& config)
    : m_config(config),
      m_sdl_context(config.graphics_config)
      , m_timer(config.graphics_config.target_fps, config.graphics_config.vsync_enabled)
      , m_input()
      , m_assets(m_sdl_context.get_renderer())
      , m_physics(config.physics_config)
      , m_render_queue()
      , m_entity_spawner(*this) {
}

void App::run() {
    bool is_running = true;
    std::vector<Entity*> entities;
    std::vector<Entity*> ticking_entities;

    while (is_running) {
        m_timer.frame_start();

        // Check for quit
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        m_entity_spawner.resolve_requests();
        m_entity_spawner.get_entities(entities);
        m_entity_spawner.get_ticking_entities(ticking_entities);

        const float delta_time = m_timer.get_delta_time();
        const float scaled_delta_time = delta_time * m_timer.get_time_scale();

        // input.tick()
        const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);
        m_input.tick(scaled_delta_time, keyboard_state);

        // entities.tick()
        for (Entity* entity : ticking_entities) {
            entity->tick(scaled_delta_time);
        }

        // entities.physics_tick()
        m_physics.tick_entities(scaled_delta_time, ticking_entities);

        // entities.render_tick()
        for (Entity* entity : entities) {
            entity->render_tick(scaled_delta_time, m_render_queue);
        }

        render_frame();

        m_timer.frame_end();
    }
}

bool App::is_initialized() const {
    return m_sdl_context.is_initialized();
}

const AppConfig& App::get_config() const {
    return m_config;
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

void App::render_frame() {
    SDL_SetRenderDrawBlendMode(m_sdl_context.get_renderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_sdl_context.get_renderer(), 43, 47, 119, 255);
    SDL_RenderClear(m_sdl_context.get_renderer());

    // Render entities
    Entity* camera_entity = m_entity_spawner.get_camera_entity();
    CameraComponent* camera_component = camera_entity->get_component<CameraComponent>();
    TransformComponent* camera_transform = camera_entity->get_transform();
    Vector2 camera_position = Vector2::lerp(camera_transform->get_prev_position(), camera_transform->get_position(),
                                            m_physics.get_interpolation_fraction());

    for (const RenderData& render_data : m_render_queue.get_render_data()) {
        SDL_Texture* texture = m_assets.get_texture(render_data.texture_id);
        int texture_width = 0;
        int texture_height = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);

        Vector2 world_position = Vector2::lerp(
            render_data.prev_position, render_data.position, m_physics.get_interpolation_fraction());

        Vector2 screen_position = camera_component->world_to_screen(world_position, camera_position);

        SDL_FRect dst{
            screen_position.x,
            screen_position.y,
            static_cast<float>(texture_width) * render_data.scale.x,
            static_cast<float>(texture_height) * render_data.scale.y,
        };

        SDL_RenderCopyF(m_sdl_context.get_renderer(), texture, nullptr, &dst);
    }

    m_render_queue.clear();

    // Render debug draws
    debug::render_debug_draws(m_sdl_context.get_renderer(), m_assets.get_white_pixel_texture(), camera_component);

    SDL_RenderPresent(m_sdl_context.get_renderer());
}
