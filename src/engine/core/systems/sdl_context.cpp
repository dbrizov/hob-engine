#include "sdl_context.h"

#include <SDL3/SDL.h>

#include "engine/core/engine_config.h"
#include "engine/core/debug.h"

namespace hob {
    SdlContext::SdlContext(const GraphicsConfig& graphics_config) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            debug::log_error("SDL_Init Error: {}", SDL_GetError());
            return;
        }

        debug::log("SDL_Init");

        m_window = SDL_CreateWindow(
            graphics_config.window_title.c_str(),
            static_cast<int>(graphics_config.window_width),
            static_cast<int>(graphics_config.window_height),
            0);

        if (!m_window) {
            debug::log_error("SDL_CreateWindow Error: {}", SDL_GetError());
            SDL_Quit();
            return;
        }

        debug::log("SDL_CreateWindow");

        const SDL_GPUShaderFormat shader_formats =
            SDL_GPU_SHADERFORMAT_SPIRV |
            SDL_GPU_SHADERFORMAT_DXIL |
            SDL_GPU_SHADERFORMAT_MSL;

#ifndef NDEBUG
        const bool debug_mode = true;
#else
        const bool debug_mode = false;
#endif

        m_gpu_device = SDL_CreateGPUDevice(shader_formats, debug_mode, nullptr);
        if (!m_gpu_device) {
            debug::log_error("SDL_CreateGPUDevice Error: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            SDL_Quit();
            return;
        }

        debug::log("SDL_CreateGPUDevice ({})", SDL_GetGPUDeviceDriver(m_gpu_device));

        if (!SDL_ClaimWindowForGPUDevice(m_gpu_device, m_window)) {
            debug::log_error("SDL_ClaimWindowForGPUDevice Error: {}", SDL_GetError());
            SDL_DestroyGPUDevice(m_gpu_device);
            m_gpu_device = nullptr;
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            SDL_Quit();
            return;
        }

        debug::log("SDL_ClaimWindowForGPUDevice");

        const SDL_GPUPresentMode present_mode = graphics_config.vsync_enabled
                                                    ? SDL_GPU_PRESENTMODE_VSYNC
                                                    : SDL_GPU_PRESENTMODE_MAILBOX;

        if (!SDL_SetGPUSwapchainParameters(m_gpu_device,
                                           m_window,
                                           SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                           present_mode)) {
            // MAILBOX may not be supported on all drivers; fall back to VSYNC, which is mandatory.
            SDL_SetGPUSwapchainParameters(m_gpu_device,
                                          m_window,
                                          SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                          SDL_GPU_PRESENTMODE_VSYNC);
        }

        m_is_initialized = true;
    }

    SdlContext::~SdlContext() {
        if (m_gpu_device && m_window) {
            SDL_ReleaseWindowFromGPUDevice(m_gpu_device, m_window);
            debug::log("SDL_ReleaseWindowFromGPUDevice");
        }

        if (m_gpu_device) {
            SDL_DestroyGPUDevice(m_gpu_device);
            m_gpu_device = nullptr;
            debug::log("SDL_DestroyGPUDevice");
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            debug::log("SDL_DestroyWindow");
        }

        SDL_Quit();
        debug::log("SDL_Quit");
    }

    bool SdlContext::is_initialized() const {
        return m_is_initialized;
    }

    SDL_Window* SdlContext::get_window() const {
        return m_window;
    }

    SDL_GPUDevice* SdlContext::get_gpu_device() const {
        return m_gpu_device;
    }

    Vector2 SdlContext::get_window_size() const {
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(m_window, &width, &height);
        return Vector2(static_cast<float>(width), static_cast<float>(height));
    }
}
