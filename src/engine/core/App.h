#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H
#include "Assets.h"
#include "Input.h"
#include "Render.h"
#include "SdlContext.h"
#include "Timer.h"
#include "engine/entity/EntitySpawner.h"


struct SDL_Window;
struct SDL_Renderer;


class App {
    // SDL context must be declared first so it is destroyed last.
    // Objects below depend on SDL resources.
    SdlContext m_sdl_context;
    Timer m_timer;
    Input m_input;
    Assets m_assets;
    RenderQueue m_render_queue;
    EntitySpawner m_entity_spawner;

public:
    App(uint32_t fps,
        uint32_t screen_width,
        uint32_t screen_height,
        const std::string& window_title,
        const std::filesystem::path& input_config_path,
        const std::filesystem::path& assets_root_path);

    void run();
    bool is_initialized() const;
    EntitySpawner* get_entity_spawner();
};


#endif //CPP_PLATFORMER_APP_H
