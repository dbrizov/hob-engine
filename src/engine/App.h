#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H
#include "Assets.h"
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
    std::filesystem::path input_config_path;
    std::filesystem::path assets_root_path;

    AppConfig(
        uint32_t fps_,
        uint32_t screen_width_,
        uint32_t screen_height_,
        const std::string& window_title_,
        const std::filesystem::path& input_config_path_,
        const std::filesystem::path& assets_root_path_)
        : fps(fps_)
        , screen_width(screen_width_)
        , screen_height(screen_height_)
        , window_title(window_title_)
        , input_config_path(input_config_path_)
        , assets_root_path(assets_root_path_)
    {};
};


class App {
private:
    AppConfig m_config;
    Timer m_timer;
    Input m_input;
    Assets m_assets;
    EntitySpawner m_entity_spawner;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

public:
    explicit App(const AppConfig& config);
    ~App();

    bool init();
    void run();

    EntitySpawner* get_entity_spawner();
};


#endif //CPP_PLATFORMER_APP_H
