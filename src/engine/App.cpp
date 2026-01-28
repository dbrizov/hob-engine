#include "App.h"

#include <SDL.h>
#include <SDL_image.h>
#include <fmt/printf.h>

#include "Timer.h"

App::App(AppConfig config)
    : m_config(config)
    , m_timer(config.fps)
    , m_sdl_window(nullptr)
    , m_sdl_renderer(nullptr) {}

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
        fmt::println("SDL_Init Error: {}", SDL_GetError());
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fmt::println("IMG_Init Error: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Create window
    m_sdl_window = SDL_CreateWindow(
        m_config.window_title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_config.screen_width),
        static_cast<int>(m_config.screen_height),
        SDL_WINDOW_SHOWN);

    if (!m_sdl_window) {
        fmt::println("SDL_CreateWindow Error: {}", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Create renderer
    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_sdl_renderer) {
        fmt::println("SDL_CreateRenderer Error: {}", SDL_GetError());
        SDL_DestroyWindow(m_sdl_window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void App::run() {
    bool is_running = true;
    SDL_Event event;

    while (is_running) {
        // --- Event handling ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    is_running = false;
                }
            }
        }

        m_timer.tick();

        const float delta_time = m_timer.get_delta_time();
        const float scaled_delta_time = delta_time * m_timer.get_time_scale();

        // --- Rendering ---
        SDL_SetRenderDrawColor(m_sdl_renderer, 30, 30, 60, 255); // dark blue background
        SDL_RenderClear(m_sdl_renderer);

        // Example: draw a simple white square for the player
        SDL_Rect playerRect;
        playerRect.x = 50;
        playerRect.y = 50;
        playerRect.w = 50;
        playerRect.h = 50;
        SDL_SetRenderDrawColor(m_sdl_renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(m_sdl_renderer, &playerRect);

        SDL_RenderPresent(m_sdl_renderer);
    }
}
