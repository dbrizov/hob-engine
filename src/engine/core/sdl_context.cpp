#include "sdl_context.h"

#include <SDL3/SDL.h>
#include <fmt/base.h>

#include "app.h"

namespace hob {
    SdlContext::SdlContext(const GraphicsConfig& graphics_config) {
        // SDL_Init
        int sld_init_flags = SDL_INIT_VIDEO;
        if (!SDL_Init(sld_init_flags)) {
            fmt::println(stderr, "SDL_Init Error: {}", SDL_GetError());
            return;
        }

        fmt::println("SDL_Init");

        // SDL_CreateWindow
        SDL_WindowFlags window_flags = 0;
        m_window = SDL_CreateWindow(
            graphics_config.window_title.c_str(),
            static_cast<int>(graphics_config.window_width),
            static_cast<int>(graphics_config.window_height),
            window_flags);

        if (!m_window) {
            fmt::println(stderr, "SDL_CreateWindow Error: {}", SDL_GetError());
            SDL_Quit();
            return;
        }

        fmt::println("SDL_CreateWindow");

        // Create renderer
        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer) {
            fmt::println(stderr, "SDL_CreateRenderer Error: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return;
        }

        int vsync = graphics_config.vsync_enabled ? 1 : 0;
        SDL_SetRenderVSync(m_renderer, vsync);

        SDL_SetRenderLogicalPresentation(
            m_renderer,
            static_cast<int>(graphics_config.logical_resolution_width),
            static_cast<int>(graphics_config.logical_resolution_height),
            SDL_LOGICAL_PRESENTATION_STRETCH);

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
