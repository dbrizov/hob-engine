#ifndef CPP_PLATFORMER_SDLCONTEXT_H
#define CPP_PLATFORMER_SDLCONTEXT_H
#include <cstdint>
#include <string>


struct SDL_Window;
struct SDL_Renderer;


class SdlContext {
private:
    bool m_is_initialized;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    uint32_t m_screen_width = 0;
    uint32_t m_screen_height = 0;
    std::string m_window_title;

public:
    SdlContext(uint32_t screen_width, uint32_t screen_height, const std::string& window_title);
    ~SdlContext();

    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;

    SdlContext(SdlContext&&) = delete;
    SdlContext& operator=(SdlContext&&) = delete;

    bool is_initialized() const;
    SDL_Window* get_window() const;
    SDL_Renderer* get_renderer() const;
    uint32_t get_screen_width() const;
    uint32_t get_screen_height() const;
};


#endif //CPP_PLATFORMER_SDLCONTEXT_H
