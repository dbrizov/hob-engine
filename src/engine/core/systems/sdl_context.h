#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "engine/math/vector2.h"

namespace hob {
    struct GraphicsConfig;

    class SdlContext {
        bool m_is_initialized = false;
        SDL_Window* m_window = nullptr;
        SDL_GPUDevice* m_gpu_device = nullptr;

    public:
        explicit SdlContext(const GraphicsConfig& graphics_config);
        ~SdlContext();

        SdlContext(const SdlContext&) = delete;
        SdlContext& operator=(const SdlContext&) = delete;

        SdlContext(SdlContext&&) = delete;
        SdlContext& operator=(SdlContext&&) = delete;

        bool is_initialized() const;
        SDL_Window* get_window() const;
        SDL_GPUDevice* get_gpu_device() const;

        Vector2 get_window_size() const;
        float get_dpi_scale() const;
    };
}
