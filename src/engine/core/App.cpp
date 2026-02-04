#include "App.h"

#include <SDL.h>

#include "Timer.h"

App::App(uint32_t target_fps,
         bool vsync_enabled,
         const std::string& window_title,
         uint32_t window_width,
         uint32_t window_height)
    : m_sdl_context(vsync_enabled, window_title, window_width, window_height)
      , m_timer(target_fps, vsync_enabled)
      , m_input()
      , m_assets(m_sdl_context.get_renderer())
      , m_render_queue()
      , m_entity_spawner() {
    m_entity_spawner.set_app(this);
}

void App::run() {
    bool is_running = true;

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

        const float delta_time = m_timer.get_delta_time();
        const float scaled_delta_time = delta_time * m_timer.get_time_scale();

        // input.tick()
        const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);
        m_input.tick(delta_time, keyboard_state);

        // entities.tick()
        for (Entity* entity : m_entity_spawner.get_entities()) {
            if (entity->is_ticking()) {
                entity->tick(scaled_delta_time);
            }
        }

        // entities.physics_tick()

        // entities.render_tick()
        for (Entity* entity : m_entity_spawner.get_entities()) {
            entity->render_tick(delta_time, m_render_queue);
        }

        // --- Rendering ---
        SDL_SetRenderDrawColor(m_sdl_context.get_renderer(), 14, 219, 248, 255);
        SDL_RenderClear(m_sdl_context.get_renderer());

        for (const RenderData& render_data : m_render_queue.get_render_data()) {
            SDL_Texture* texture = m_assets.get_texture(render_data.texture_id);
            int texture_width = 0;
            int texture_height = 0;
            SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);

            SDL_FRect dst{
                render_data.position.x,
                render_data.position.y,
                static_cast<float>(texture_width) * render_data.scale.x,
                static_cast<float>(texture_height) * render_data.scale.y,
            };

            SDL_RenderCopyF(m_sdl_context.get_renderer(), texture, nullptr, &dst);
        }

        m_render_queue.clear();

        SDL_RenderPresent(m_sdl_context.get_renderer());

        m_timer.frame_end();
    }
}

bool App::is_initialized() const {
    return m_sdl_context.is_initialized();
}

Input* App::get_input() {
    return &m_input;
}

Assets* App::get_assets() {
    return &m_assets;
}

EntitySpawner* App::get_entity_spawner() {
    return &m_entity_spawner;
}
