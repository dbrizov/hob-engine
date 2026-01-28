#include "App.h"

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

App::App(int fps, int screen_width, int screen_height, const char* window_title) {
    m_fps = fps;
    m_screen_width = screen_width;
    m_screen_height = screen_height;
    m_window_title = window_title;
    m_sdl_window = nullptr;
    m_sdl_renderer = nullptr;
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
    // Init SDL subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    // Create window
    m_sdl_window = SDL_CreateWindow(
        m_window_title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_screen_width,
        m_screen_height,
        SDL_WINDOW_SHOWN);

    if (!m_sdl_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    // Create renderer
    m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_sdl_renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(m_sdl_window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void App::run() {
    bool is_running = true;
    const int frame_delay = 1000 / m_fps;
    SDL_Event event;

    while (is_running) {
        Uint32 frame_start = SDL_GetTicks();

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

        // --- Frame limiter ---
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
    }
}
