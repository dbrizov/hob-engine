#include "App.h"

#include <SDL.h>
#include <fmt/base.h>

#include "Timer.h"

App::App(uint32_t fps,
         uint32_t screen_width,
         uint32_t screen_height,
         const std::string& window_title,
         const std::filesystem::path& input_config_path,
         const std::filesystem::path& assets_root_path)
    : m_sdl_context(screen_width, screen_height, window_title)
    , m_timer(fps)
    , m_input(input_config_path)
    , m_assets(assets_root_path, m_sdl_context.get_renderer())
    , m_render_queue()
    , m_entity_spawner() {}

// TODO Remove debug input events
void handle_event(const InputEvent& event) {
    fmt::println("InputEvent (name: {}, type: {}, axis_value: {})", event.name, static_cast<int>(event.type), event.axis_value);
}

void App::run() {
    m_input.add_input_event_handler(handle_event);

    bool is_running = true;
    SDL_Event event;

    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        m_timer.tick();

        m_entity_spawner.resolve_requests();

        const float delta_time = m_timer.get_delta_time();
        const float scaled_delta_time = delta_time * m_timer.get_time_scale();

        // input.tick()
        const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);
        m_input.tick(delta_time, keyboard_state);

        // entities.tick()
        for (auto& entity : m_entity_spawner.get_entities()) {
            if (entity.is_ticking()) {
                entity.tick(scaled_delta_time);
            }
        }

        // entities.physics_tick()

        // entities.render_tick()
        for (auto& entity : m_entity_spawner.get_entities()) {
            entity.render_tick(delta_time, m_render_queue);
        }

        // --- Rendering ---
        SDL_SetRenderDrawColor(m_sdl_context.get_renderer(), 14, 219, 248, 255);
        SDL_RenderClear(m_sdl_context.get_renderer());
        for (const RenderData& render_data : m_render_queue.get_render_data()) {
            SDL_Texture* tex = m_assets.get_texture(render_data.texture_id);
            int tex_w = 0, tex_h = 0;
            SDL_QueryTexture(tex, nullptr, nullptr, &tex_w, &tex_h);

            SDL_Rect dst{
                static_cast<int>(render_data.position.x),
                static_cast<int>(render_data.position.y),
                tex_w * static_cast<int>(render_data.scale.x),
                tex_h * static_cast<int>(render_data.scale.y),
            };

            SDL_RenderCopy(m_sdl_context.get_renderer(), tex, nullptr, &dst);
        }

        m_render_queue.clear();

        SDL_RenderPresent(m_sdl_context.get_renderer());
    }
}

bool App::is_initialized() const {
    return m_sdl_context.is_initialized();
}

Assets& App::get_assets() {
    return m_assets;
}

EntitySpawner& App::get_entity_spawner() {
    return m_entity_spawner;
}
