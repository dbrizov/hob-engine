#include <filesystem>

#include "engine/components/ImageComponent.h"
#include "engine/components/InputComponent.h"
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
constexpr bool PHYSICS_INTERPOLATION = true;

Entity* spawn_player_entity(App& app) {
    Entity* entity = app.get_entity_spawner()->spawn_entity();
    entity->set_is_ticking(true);

    entity->add_component<InputComponent>();
    entity->add_component<PlayerComponent>();

    ImageComponent* image_component = entity->add_component<ImageComponent>();
    const std::filesystem::path path =
        PathUtils::get_assets_root_path() / "images" / "entities" / "player" / "idle" / "00.png";
    const TextureId texture_id = app.get_assets()->load_texture(path);
    image_component->set_texture_id(texture_id);

    return entity;
}

Entity* spawn_enemy_entity(App& app) {
    Entity* entity = app.get_entity_spawner()->spawn_entity();

    TransformComponent* transform = entity->get_transform();
    transform->set_position(Vector2(100.0f, 100.0f));

    ImageComponent* image_component = entity->add_component<ImageComponent>();
    const std::filesystem::path path =
        PathUtils::get_assets_root_path() / "images" / "entities" / "enemy" / "idle" / "00.png";
    const TextureId texture_id = app.get_assets()->load_texture(path);
    image_component->set_texture_id(texture_id);

    return entity;
}

int main(int argc, char* argv[]) {
    App app(
        WINDOW_TITLE,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        LOGICAL_RESOLUTION_WIDTH,
        LOGICAL_RESOLUTION_HEIGHT,
        TARGET_FPS,
        VSYNC_ENABLED,
        PHYSICS_TICKS_PER_SECOND,
        PHYSICS_INTERPOLATION);

    if (!app.is_initialized()) {
        return 1;
    }

    spawn_player_entity(app);
    spawn_enemy_entity(app);

    app.run();

    return 0;
}
