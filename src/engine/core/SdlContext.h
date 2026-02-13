#ifndef HOB_ENGINE_SDLCONTEXT_H
#define HOB_ENGINE_SDLCONTEXT_H
#include <cstdint>
#include <string>


struct SDL_Window;
struct SDL_Renderer;


class SdlContext {
    bool m_is_initialized;
    std::string m_window_title;
    uint32_t m_window_width;
    uint32_t m_window_height;
    uint32_t m_logical_resolution_width;
    uint32_t m_logical_resolution_height;
    bool m_vsync_enabled;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

public:
    SdlContext(const std::string& window_title,
               uint32_t window_width,
               uint32_t window_height,
               uint32_t logical_resolution_width,
               uint32_t logical_resolution_height,
               bool vsync_enabled);

    ~SdlContext();

    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;

    SdlContext(SdlContext&&) = delete;
    SdlContext& operator=(SdlContext&&) = delete;

    bool is_initialized() const;
    SDL_Window* get_window() const;
    SDL_Renderer* get_renderer() const;
};


#endif //HOB_ENGINE_SDLCONTEXT_H
