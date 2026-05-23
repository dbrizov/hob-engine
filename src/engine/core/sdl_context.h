#pragma once

#include <SDL3/SDL_video.h>

namespace hob {
    struct GraphicsConfig;

    class SdlContext {
        bool m_is_initialized = false;
        SDL_Window* m_window = nullptr;
        SDL_GLContext m_gl_context = nullptr;

    public:
        explicit SdlContext(const GraphicsConfig& graphics_config);
        ~SdlContext();

        SdlContext(const SdlContext&) = delete;
        SdlContext& operator=(const SdlContext&) = delete;

        SdlContext(SdlContext&&) = delete;
        SdlContext& operator=(SdlContext&&) = delete;

        bool is_initialized() const;
        SDL_Window* get_window() const;
        SDL_GLContext get_gl_context() const;

        void swap();
    };
}
