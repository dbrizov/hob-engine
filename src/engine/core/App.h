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
    App(uint32_t target_fps,
        bool vsync_enabled,
        const std::string& window_title,
        uint32_t window_width,
        uint32_t window_height);

    void run();
    bool is_initialized() const;

    Input* get_input();
    Assets* get_assets();
    EntitySpawner* get_entity_spawner();

private:
    void input_tick(float delta_time);
    void entities_tick(float scaled_delta_time);
    void entities_physics_tick(float scaled_delta_time);
    void entities_render_tick(float delta_time);
    void render_frame();
};


#endif //CPP_PLATFORMER_APP_H
