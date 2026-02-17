#include <filesystem>

#include "engine/components/BoxColliderComponent.h"
#include "engine/components/CharacterBodyComponent.h"
#include "engine/components/ImageComponent.h"
#include "engine/components/InputComponent.h"
#include "engine/components/RigidbodyComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"
#include "engine/core/PathUtils.h"
#include "engine/entity/Entity.h"
#include "game/PlayerComponent.h"

constexpr std::string WINDOW_TITLE = "SDL2 Window";
constexpr uint32_t WINDOW_WIDTH = 1152;
constexpr uint32_t WINDOW_HEIGHT = 648;
constexpr uint32_t LOGICAL_RESOLUTION_WIDTH = WINDOW_WIDTH;
constexpr uint32_t LOGICAL_RESOLUTION_HEIGHT = WINDOW_HEIGHT;
constexpr uint32_t TARGET_FPS = 60;
constexpr bool VSYNC_ENABLED = true;
constexpr float PIXELS_PER_METER = 64.0f;

const Vector2 PHYSICS_GRAVITY = Vector2(0.0f, -9.81f);
constexpr uint32_t PHYSICS_TICKS_PER_SECOND = 60;
constexpr uint32_t PHYSICS_SUB_STEPS_PER_TICK = 4;
constexpr bool PHYSICS_INTERPOLATION_ENABLED = false;

Entity& spawn_player_entity(App& app, const Vector2& position) {
    Entity& entity = app.get_entity_spawner().spawn_entity();
    entity.set_ticking(true);
    entity.get_transform()->set_position(position);

    entity.add_component<InputComponent>();
    entity.add_component<PlayerComponent>();
    entity.add_component<CharacterBodyComponent>();

    // ImageComponent* image_component = entity.add_component<ImageComponent>();
    // const std::filesystem::path path =
    //     PathUtils::get_assets_root_path() / "images" / "entities" / "player" / "idle" / "00.png";
    // const TextureId texture_id = app.get_assets().load_texture(path);
    // image_component->set_texture_id(texture_id);

    return entity;
}

Entity& spawn_static_box(App& app, const Vector2& position, float rotation_degrees) {
    Entity& entity = app.get_entity_spawner().spawn_entity();
    entity.get_transform()->set_position(position);
    entity.get_transform()->set_rotation_degrees(rotation_degrees);

    entity.add_component<RigidbodyComponent>();
    entity.add_component<BoxColliderComponent>();

    return entity;
}

Entity& spawn_dynamic_box(App& app, const Vector2& position, float rotation_degrees) {
    Entity& entity = app.get_entity_spawner().spawn_entity();
    entity.set_ticking(true);
    entity.get_transform()->set_position(position);
    entity.get_transform()->set_rotation_degrees(rotation_degrees);

    RigidbodyComponent* rigidbody = entity.add_component<RigidbodyComponent>();
    rigidbody->set_body_type(BodyType::DYNAMIC);

    entity.add_component<BoxColliderComponent>();

    return entity;
}

int main(int argc, char* argv[]) {
    GraphicsConfig graphics_config;
    graphics_config.window_title = WINDOW_TITLE;
    graphics_config.window_width = WINDOW_WIDTH;
    graphics_config.window_height = WINDOW_HEIGHT;
    graphics_config.logical_resolution_width = LOGICAL_RESOLUTION_WIDTH;
    graphics_config.logical_resolution_height = LOGICAL_RESOLUTION_HEIGHT;
    graphics_config.target_fps = TARGET_FPS;
    graphics_config.vsync_enabled = VSYNC_ENABLED;
    graphics_config.pixels_per_meter = PIXELS_PER_METER;

    PhysicsConfig physics_config;
    physics_config.gravity = PHYSICS_GRAVITY;
    physics_config.ticks_per_second = PHYSICS_TICKS_PER_SECOND;
    physics_config.sub_steps_per_tick = PHYSICS_SUB_STEPS_PER_TICK;
    physics_config.interpolation_enabled = PHYSICS_INTERPOLATION_ENABLED;

    AppConfig app_config;
    app_config.graphics_config = graphics_config;
    app_config.physics_config = physics_config;

    App app(app_config);

    if (!app.is_initialized()) {
        return 1;
    }

    spawn_player_entity(app, Vector2(0.0f, 0.0f));

    // floor
    spawn_static_box(app, Vector2(-4.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(-3.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(-2.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(-1.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(0.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(1.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(2.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(3.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(4.0f, -3.0f), 0.0f);

    // stairs
    spawn_static_box(app, Vector2(5.0f, -2.7f), 0.0f);
    spawn_static_box(app, Vector2(6.0f, -2.4f), 0.0f);
    spawn_static_box(app, Vector2(7.0f, -2.1f), 0.0f);
    spawn_static_box(app, Vector2(8.0f, -1.8f), 0.0f);
    spawn_static_box(app, Vector2(9.0f, -1.5f), 0.0f);
    spawn_static_box(app, Vector2(10.0f, -1.2f), 0.0f);
    spawn_static_box(app, Vector2(11.0f, -0.9f), 0.0f);
    spawn_static_box(app, Vector2(12.0f, -0.6f), 0.0f);
    spawn_static_box(app, Vector2(13.0f, -0.3f), 0.0f);

    // left wall
    spawn_static_box(app, Vector2(-5.0f, -3.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, -2.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, 0.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, 1.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, 2.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, 3.0f), 0.0f);
    spawn_static_box(app, Vector2(-5.0f, 4.0f), 0.0f);

    // dynamic boxes
    spawn_dynamic_box(app, Vector2(-3.0f, 3.0f), 60.0f);
    spawn_dynamic_box(app, Vector2(-2.0f, 0.0f), 0.0f);
    spawn_dynamic_box(app, Vector2(1.0f, 1.5f), 30.0f);
    spawn_dynamic_box(app, Vector2(0.0f, 3.0f), 40.0f);
    spawn_dynamic_box(app, Vector2(1.0f, 4.5f), 0.0f);
    spawn_dynamic_box(app, Vector2(2.0f, 6.0f), -10.0f);

    app.run();

    return 0;
}
