#ifndef HOB_ENGINE_APP_H
#define HOB_ENGINE_APP_H
#include "Assets.h"
#include "Input.h"
#include "Physics.h"
#include "Render.h"
#include "SdlContext.h"
#include "Timer.h"
#include "engine/entity/EntitySpawner.h"


struct SDL_Window;
struct SDL_Renderer;


struct AppConfig {
    std::string window_title = "SDL2 Window";
    uint32_t window_width = 1152;
    uint32_t window_height = 648;
    uint32_t logical_resolution_width = 1152;
    uint32_t logical_resolution_height = 648;
    uint32_t target_fps = 60;
    bool vsync_enabled = true;
    uint32_t physics_ticks_per_second = 60;
    uint32_t physics_sub_steps_per_tick = 4;
    bool physics_interpolation = true;
    float pixels_per_meter = 100.0f;
};


class App {
    AppConfig m_config;

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
    explicit App(const AppConfig& config);

    void run();

    bool is_initialized() const;

    const AppConfig& get_config() const;

    Timer& get_timer();
    Input& get_input();
    Assets& get_assets();
    Physics& get_physics();
    EntitySpawner& get_entity_spawner();

private:
    void render_frame();
};


#endif //HOB_ENGINE_APP_H
