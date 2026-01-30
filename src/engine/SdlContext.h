#ifndef CPP_PLATFORMER_SDLCONTEXT_H
#define CPP_PLATFORMER_SDLCONTEXT_H
#include <cstdint>
#include <string>


struct SDL_Window;
struct SDL_Renderer;


class SdlContext {
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    uint32_t m_screen_width = 0;
    uint32_t m_screen_height = 0;
    std::string m_window_title;

public:
    explicit SdlContext(uint32_t screen_width, uint32_t screen_height, const std::string& window_title);
    ~SdlContext();

    SDL_Window* get_window() const;
    SDL_Renderer* get_renderer() const;
    uint32_t get_screen_width() const;
    uint32_t get_screen_height() const;
};


#endif //CPP_PLATFORMER_SDLCONTEXT_H
