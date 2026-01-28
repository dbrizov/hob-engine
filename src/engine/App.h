#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H
#include "Timer.h"


struct SDL_Window;
struct SDL_Renderer;


struct AppConfig {
    uint32_t fps = 0;
    uint32_t screen_width = 0;
    uint32_t screen_height = 0;
    const char* window_title = nullptr;

    AppConfig& set_fps(uint32_t fps) {
        this->fps = fps;
        return *this;
    }

    AppConfig& set_screen_width(uint32_t screen_width) {
        this->screen_width = screen_width;
        return *this;
    }

    AppConfig& set_screen_height(uint32_t screen_height) {
        this->screen_height = screen_height;
        return *this;
    }

    AppConfig& set_window_title(const char* window_title) {
        this->window_title = window_title;
        return *this;
    }
};

class App {
private:
    AppConfig m_config;
    Timer m_timer;
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_renderer;

public:
    App(AppConfig config);
    ~App();

    bool init();
    void run();
};


#endif //CPP_PLATFORMER_APP_H
