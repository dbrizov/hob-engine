#ifndef CPP_PLATFORMER_SDLCONTEXT_H
#define CPP_PLATFORMER_SDLCONTEXT_H
#include <cstdint>
#include <string>


struct SDL_Window;
struct SDL_Renderer;


class SdlContext {
    bool m_is_initialized;
    bool m_vsync_enabled;
    std::string m_window_title;
    uint32_t m_window_width;
    uint32_t m_window_height;
    uint32_t m_logical_resolution_width;
    uint32_t m_logical_resolution_height;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

public:
    SdlContext(bool vsync_enabled,
               const std::string& window_title,
               uint32_t window_width,
               uint32_t window_height,
               uint32_t logical_resolution_width,
               uint32_t logical_resolution_height);

    ~SdlContext();

    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;

    SdlContext(SdlContext&&) = delete;
    SdlContext& operator=(SdlContext&&) = delete;

    bool is_initialized() const;
    SDL_Window* get_window() const;
    SDL_Renderer* get_renderer() const;
};


#endif //CPP_PLATFORMER_SDLCONTEXT_H
