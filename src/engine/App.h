#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H
#include "Input.h"
#include "Timer.h"
#include "entity/EntitySpawner.h"


struct SDL_Window;
struct SDL_Renderer;


struct AppConfig {
    uint32_t fps = 0;
    uint32_t screen_width = 0;
    uint32_t screen_height = 0;
    std::string window_title;
    std::string input_config_path;

    AppConfig& set_fps(uint32_t fps_) {
        fps = fps_;
        return *this;
    }

    AppConfig& set_screen_width(uint32_t screen_width_) {
        screen_width = screen_width_;
        return *this;
    }

    AppConfig& set_screen_height(uint32_t screen_height_) {
        screen_height = screen_height_;
        return *this;
    }

    AppConfig& set_window_title(std::string window_title_) {
        window_title = std::move(window_title_);
        return *this;
    }

    AppConfig& set_input_config_path(std::string input_config_path_) {
        input_config_path = std::move(input_config_path_);
        return *this;
    }
};


class App {
private:
    AppConfig m_config;
    Timer m_timer;
    Input m_input;
    EntitySpawner m_entity_spawner;
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_renderer;

public:
    explicit App(const AppConfig& config);
    ~App();

    bool init();
    void run();

    EntitySpawner* get_entity_spawner();
};


#endif //CPP_PLATFORMER_APP_H
