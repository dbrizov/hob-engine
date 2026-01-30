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
    , m_assets(config.assets_root_path)
    , m_entity_spawner()
    , m_window(nullptr)
    , m_renderer(nullptr) {
}

App::~App() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
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
    m_window = SDL_CreateWindow(
        m_config.window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_config.screen_width),
        static_cast<int>(m_config.screen_height),
        SDL_WINDOW_SHOWN);

    if (!m_window) {
        fmt::println(stderr, "SDL_CreateWindow Error: {}", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    fmt::println("SDL_CreateWindow (Success)");

    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        fmt::println(stderr, "SDL_CreateRenderer Error: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
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

    // Debug load_texture
    const std::filesystem::path path = m_assets.get_assets_root_path() / "images" / "entities" / "player" / "idle" / "00.png";
    fmt::println("texture_path: '{}'", path.string());
    const TextureId texture_id = m_assets.load_texture(m_renderer, path.c_str());
    SDL_Texture* texture = m_assets.get_texture(texture_id);

    int tex_w = 0, tex_h = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &tex_w, &tex_h);

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
        SDL_SetRenderDrawColor(m_renderer, 14, 219, 248, 255);
        SDL_RenderClear(m_renderer);

        SDL_Rect destination;
        destination.w = tex_w * 2;
        destination.h = tex_h * 2;
        destination.x = (m_config.screen_width - destination.w) / 2;
        destination.y = (m_config.screen_height - destination.h) / 2;

        SDL_RenderCopy(m_renderer, texture, nullptr, &destination);

        // entities.render_tick()

        SDL_RenderPresent(m_renderer);
    }
}

EntitySpawner* App::get_entity_spawner() {
    return &m_entity_spawner;
}
