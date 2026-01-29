#include "App.h"

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <fmt/base.h>

#include "Timer.h"

App::App(const AppConfig& config)
    : m_config(config)
    , m_timer(config.fps)
    , m_input(config.input_config_path)
    , m_entity_spawner()
    , m_sdl_window(nullptr)
    , m_sdl_renderer(nullptr) {
}

App::~App() {
    if (m_sdl_renderer) {
        SDL_DestroyRenderer(m_sdl_renderer);
    }

    if (m_sdl_window) {
        SDL_DestroyWindow(m_sdl_window);
    }

    IMG_Quit();
    SDL_Quit();
}

bool App::init() {
    // Init SDL sub-systems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fmt::println(stderr, "SDL_Init Error: {}", SDL_GetError());
        return false;
    }

    fmt::println("SDL_Init (Success)");

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fmt::println(stderr, "IMG_Init Error: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    fmt::println("IMG_Init (Success)");

    // Create window
    m_sdl_window = SDL_CreateWindow(
        m_config.window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_config.screen_width),
        static_cast<int>(m_config.screen_height),
        SDL_WINDOW_SHOWN);

    if (!m_sdl_window) {
        fmt::println(stderr, "SDL_CreateWindow Error: {}", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    fmt::println("SDL_CreateWindow (Success)");

    // Create renderer
    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_sdl_renderer) {
        fmt::println(stderr, "SDL_CreateRenderer Error: {}", SDL_GetError());
        SDL_DestroyWindow(m_sdl_window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    fmt::println("SDL_CreateRenderer (Success)");

    return true;
}

void handle_event(const InputEvent& event) {
    fmt::println("InputEvent (name: {}, type: {}, axis_value: {})", event.name, static_cast<int>(event.type), event.axis_value);
}

void App::run() {
    m_input.subscribe(handle_event);

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

        // --- Rendering ---
        SDL_SetRenderDrawColor(m_sdl_renderer, 14, 219, 248, 255);
        SDL_RenderClear(m_sdl_renderer);

        // entities.render_tick()

        SDL_RenderPresent(m_sdl_renderer);
    }
}

EntitySpawner* App::get_entity_spawner() {
    return &m_entity_spawner;
}
