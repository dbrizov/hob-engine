#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H
#include "Assets.h"
#include "Input.h"
#include "SdlContext.h"
#include "Timer.h"
#include "entity/EntitySpawner.h"


struct SDL_Window;
struct SDL_Renderer;


class App {
private:
    // Order matters. Some objects use SDL resources, and must be destroyed before the SdlContext
    SdlContext m_sdl_context;
    Timer m_timer;
    Input m_input;
    Assets m_assets;
    EntitySpawner m_entity_spawner;

public:
    App(uint32_t fps,
        uint32_t screen_width,
        uint32_t screen_height,
        const std::string& window_title,
        const std::filesystem::path& input_config_path,
        const std::filesystem::path& assets_root_path);

    void run();

    EntitySpawner* get_entity_spawner();
};


#endif //CPP_PLATFORMER_APP_H
