#include <filesystem>

#include "engine/components/BoxColliderComponent.h"
#include "engine/components/ImageComponent.h"
#include "engine/components/InputComponent.h"
#include "engine/components/RigidbodyComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"
#include "engine/core/PathUtils.h"
#include "engine/entity/Entity.h"
#include "game/PlayerComponent.h"

const std::string WINDOW_TITLE = "SDL2 Window";
constexpr uint32_t WINDOW_WIDTH = 1152;
constexpr uint32_t WINDOW_HEIGHT = 648;
constexpr uint32_t LOGICAL_RESOLUTION_WIDTH = WINDOW_WIDTH / 2;
constexpr uint32_t LOGICAL_RESOLUTION_HEIGHT = WINDOW_HEIGHT / 2;
constexpr uint32_t TARGET_FPS = 60;
constexpr bool VSYNC_ENABLED = true;
constexpr uint32_t PHYSICS_TICKS_PER_SECOND = 60;
constexpr uint32_t PHYSICS_SUB_STEPS_PER_TICK = 4;
constexpr bool PHYSICS_INTERPOLATION = true;
constexpr float PIXELS_PER_METER = 25.0f;

Entity& spawn_player_entity(App& app, const Vector2& position) {
    Entity& entity = app.get_entity_spawner().spawn_entity();
    entity.set_is_ticking(true);
    entity.get_transform()->set_position(position);

    entity.add_component<InputComponent>();
    entity.add_component<PlayerComponent>();

    ImageComponent* image_component = entity.add_component<ImageComponent>();
    const std::filesystem::path path =
        PathUtils::get_assets_root_path() / "images" / "entities" / "player" / "idle" / "00.png";
    const TextureId texture_id = app.get_assets().load_texture(path);
    image_component->set_texture_id(texture_id);

    return entity;
}

Entity& spawn_static_box(App& app, const Vector2& position, float rotation_degrees) {
    Entity& entity = app.get_entity_spawner().spawn_entity();
    entity.get_transform()->set_position(position);
    entity.get_transform()->set_rotation_degrees(rotation_degrees);

    RigidbodyComponent* rigidbody = entity.add_component<RigidbodyComponent>();
    rigidbody->set_body_type(BodyType::STATIC);

    entity.add_component<BoxColliderComponent>();

    return entity;
}

Entity& spawn_dynamic_box(App& app, const Vector2& position, float rotation_degrees) {
    Entity& entity = app.get_entity_spawner().spawn_entity();
    entity.set_is_ticking(true);
    entity.get_transform()->set_position(position);
    entity.get_transform()->set_rotation_degrees(rotation_degrees);

    RigidbodyComponent* rigidbody = entity.add_component<RigidbodyComponent>();
    rigidbody->set_body_type(BodyType::DYNAMIC);

    entity.add_component<BoxColliderComponent>();

    return entity;
}

int main(int argc, char* argv[]) {
    AppConfig app_config;
    app_config.window_title = WINDOW_TITLE;
    app_config.window_width = WINDOW_WIDTH;
    app_config.window_height = WINDOW_HEIGHT;
    app_config.logical_resolution_width = LOGICAL_RESOLUTION_WIDTH;
    app_config.logical_resolution_height = LOGICAL_RESOLUTION_HEIGHT;
    app_config.target_fps = TARGET_FPS;
    app_config.vsync_enabled = VSYNC_ENABLED;
    app_config.physics_ticks_per_second = PHYSICS_TICKS_PER_SECOND;
    app_config.physics_sub_steps_per_tick = PHYSICS_SUB_STEPS_PER_TICK;
    app_config.physics_interpolation = PHYSICS_INTERPOLATION;
    app_config.pixels_per_meter = PIXELS_PER_METER;

    App app(app_config);

    if (!app.is_initialized()) {
        return 1;
    }

    spawn_player_entity(app, Vector2(0.0f, 0.0f));

    spawn_static_box(app, Vector2(-3.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(-2.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(-1.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(0.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(1.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(2.0f, -1.0f), 0.0f);
    spawn_static_box(app, Vector2(3.0f, -1.0f), 0.0f);

    spawn_dynamic_box(app, Vector2(0.0f, 2.0f), 10.0f);

    app.run();

    return 0;
}
