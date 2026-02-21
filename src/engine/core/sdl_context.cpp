#include "sdl_context.h"

#include <SDL.h>
#include <SDL_image.h>
#include <fmt/base.h>

#include "app.h"

namespace hob {
    SdlContext::SdlContext(const GraphicsConfig& graphics_config) {
        // SDL_Init
        int sld_init_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS;
        if (SDL_Init(sld_init_flags) != 0) {
            fmt::println(stderr, "SDL_Init Error: {}", SDL_GetError());
            return;
        }

        fmt::println("SDL_Init");

        // IMG_Init
        int img_init_flags = IMG_INIT_PNG;
        if ((IMG_Init(img_init_flags) & img_init_flags) != img_init_flags) {
            fmt::println(stderr, "IMG_Init Error: {}", IMG_GetError());
            SDL_Quit();
            return;
        }

        fmt::println("IMG_Init");

        // SDL_CreateWindow
        m_window = SDL_CreateWindow(
            graphics_config.window_title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(graphics_config.window_width),
            static_cast<int>(graphics_config.window_height),
            SDL_WINDOW_SHOWN);

        if (!m_window) {
            fmt::println(stderr, "SDL_CreateWindow Error: {}", SDL_GetError());
            IMG_Quit();
            SDL_Quit();
            return;
        }

        fmt::println("SDL_CreateWindow");

        // Create renderer
        uint32_t renderer_flags = SDL_RENDERER_ACCELERATED;
        if (graphics_config.vsync_enabled) {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }

        m_renderer = SDL_CreateRenderer(m_window, -1, renderer_flags);
        if (!m_renderer) {
            fmt::println(stderr, "SDL_CreateRenderer Error: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            IMG_Quit();
            SDL_Quit();
            return;
        }

        SDL_RenderSetLogicalSize(
            m_renderer,
            static_cast<int>(graphics_config.logical_resolution_width),
            static_cast<int>(graphics_config.logical_resolution_height));

        fmt::println("SDL_CreateRenderer");

        m_is_initialized = true;
    }

    SdlContext::~SdlContext() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
            fmt::println("SDL_DestroyRenderer");
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
            fmt::println("SDL_DestroyWindow");
        }

        IMG_Quit();
        fmt::println("IMG_Quit");

        SDL_Quit();
        fmt::println("SDL_Quit");
    }

    bool SdlContext::is_initialized() const {
        return m_is_initialized;
    }

    SDL_Window* SdlContext::get_window() const {
        return m_window;
    }

    SDL_Renderer* SdlContext::get_renderer() const {
        return m_renderer;
    }

    void SdlContext::frame_start() {
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer, 43, 47, 119, 255);
        SDL_RenderClear(m_renderer);
    }

    void SdlContext::frame_end() {
        SDL_RenderPresent(m_renderer);
    }
}
