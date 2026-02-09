#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H
#include "Assets.h"
#include "Input.h"
#include "Physics.h"
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
    Physics m_physics;
    RenderQueue m_render_queue;
    EntitySpawner m_entity_spawner;

public:
    App(uint32_t target_fps,
        bool vsync_enabled,
        const std::string& window_title,
        uint32_t window_width,
        uint32_t window_height,
        uint32_t logical_resolution_width,
        uint32_t logical_resolution_height,
        uint32_t physics_ticks_per_second,
        bool physics_interpolation);

    void run();
    bool is_initialized() const;

    Timer* get_timer();
    Input* get_input();
    Assets* get_assets();
    EntitySpawner* get_entity_spawner();

private:
    void render_frame();
};


#endif //CPP_PLATFORMER_APP_H
