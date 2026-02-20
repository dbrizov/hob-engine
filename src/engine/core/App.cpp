#include "App.h"

#include <SDL.h>
#include <imgui.h>

#include "Debug.h"
#include "Timer.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/ImageComponent.h"
#include "engine/components/TransformComponent.h"

App::App(const AppConfig& config)
    : m_config(config)
      , m_sdl_context(config.graphics_config)
      , m_imgui_system(m_sdl_context.get_window(), m_sdl_context.get_renderer())
      , m_timer(config.graphics_config.target_fps, config.graphics_config.vsync_enabled)
      , m_input()
      , m_assets(m_sdl_context.get_renderer())
      , m_physics(config.physics_config)
      , m_entity_spawner(*this) {
}

void App::run() {
    bool is_running = true;
    std::vector<Entity*> entities;
    std::vector<Entity*> ticking_entities;
    std::vector<Entity*> physics_entities;
    std::vector<const Entity*> renderable_entities;

    while (is_running) {
        m_timer.frame_start();

        // - Process events for the ImGuiSystem.
        // - Check for quit.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            m_imgui_system.process_event(event);

            if (event.type == SDL_QUIT) {
                is_running = false;
            }
        }

        // - The ImGuiSystem needs to start a new frame after the events polling
        // so that it can take a valid snapshot of the current frame's events.
        // - The SdlContext's frame_start() doesn't care about events, but I call it here for consistency.
        m_sdl_context.frame_start();
        m_imgui_system.frame_start();

        m_entity_spawner.resolve_requests();
        m_entity_spawner.get_entities(entities);
        m_entity_spawner.get_ticking_entities(ticking_entities);
        m_entity_spawner.get_physics_entities(physics_entities);
        m_entity_spawner.get_renderable_entities(renderable_entities);

        const float delta_time = m_timer.get_delta_time();
        const float scaled_delta_time = delta_time * m_timer.get_time_scale();

        // input.tick()
        const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);
        m_input.tick(scaled_delta_time, keyboard_state);

        // entities.tick()
        for (Entity* entity : ticking_entities) {
            entity->tick(scaled_delta_time);
        }

        // entities.physics_tick()
        m_physics.tick_entities(scaled_delta_time, physics_entities);

#ifndef NDEBUG
        // entities.debug_draw_tick()
        for (Entity* entity : entities) {
            entity->debug_draw_tick(scaled_delta_time);
        }
#endif

        render_entities(renderable_entities);

        // TODO Remove ImGui test window
        ImGui::Begin("Test");
        ImGui::Text("Hello ImGui");
        ImGui::End();

        m_imgui_system.frame_end();
        m_sdl_context.frame_end();

        m_timer.frame_end();
    }
}

bool App::is_initialized() const {
    return m_sdl_context.is_initialized() &&
           m_imgui_system.is_initialized();
}

const AppConfig& App::get_config() const {
    return m_config;
}

Timer& App::get_timer() {
    return m_timer;
}

Input& App::get_input() {
    return m_input;
}

Assets& App::get_assets() {
    return m_assets;
}

Physics& App::get_physics() {
    return m_physics;
}

EntitySpawner& App::get_entity_spawner() {
    return m_entity_spawner;
}

void App::render_entities(const std::vector<const Entity*>& entities) {
    // Render entities
    Entity* camera_entity = m_entity_spawner.get_camera_entity();
    CameraComponent* camera_component = camera_entity->get_component<CameraComponent>();
    TransformComponent* camera_transform = camera_entity->get_transform();
    Vector2 camera_position = Vector2::lerp(camera_transform->get_prev_position(), camera_transform->get_position(),
                                            m_physics.get_interpolation_fraction());

    for (const Entity* entity : entities) {
        const TransformComponent* tr_comp = entity->get_transform();
        const ImageComponent* img_comp = entity->get_component<ImageComponent>();

        SDL_Texture* texture = m_assets.get_texture(img_comp->get_texture_id());
        int texture_width = 0;
        int texture_height = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);
        float texture_width_f = static_cast<float>(texture_width);
        float texture_height_f = static_cast<float>(texture_height);

        Vector2 img_pivot = img_comp->get_pivot();

        Vector2 tr_scale = tr_comp->get_scale();
        Vector2 img_scale = img_comp->get_scale();
        Vector2 scale = Vector2(tr_scale.x * img_scale.x, tr_scale.y * img_scale.y);

        Vector2 world_position = Vector2::lerp(
            tr_comp->get_prev_position(), tr_comp->get_position(), m_physics.get_interpolation_fraction());

        Vector2 screen_position = camera_component->world_to_screen(world_position, camera_position);
        screen_position.x -= texture_width_f * img_pivot.x * scale.x;
        screen_position.y -= texture_height_f * img_pivot.y * scale.y;

        SDL_FRect dst{
            screen_position.x,
            screen_position.y,
            texture_width_f * scale.x,
            texture_height_f * scale.y,
        };

        SDL_FPoint pivot = {
            dst.w * img_pivot.x,
            dst.h * img_pivot.y
        };

        float angle = -tr_comp->get_rotation_degrees();

        SDL_RenderCopyExF(m_sdl_context.get_renderer(), texture, nullptr, &dst, angle, &pivot, SDL_FLIP_NONE);
    }

    // Render debug draws
    debug::render_debug_draws(m_sdl_context.get_renderer(), m_assets.get_white_pixel_texture(), camera_component);
}
