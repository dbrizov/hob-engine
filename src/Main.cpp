#include <filesystem>

#include "engine/components/ImageComponent.h"
#include "engine/components/InputComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"
#include "engine/core/PathUtils.h"
#include "engine/entity/Entity.h"
#include "game/PlayerComponent.h"

constexpr uint32_t TARGET_FPS = 30;
constexpr bool VSYNC_ENABLED = true;
const std::string WINDOW_TITLE = "SDL2 Window";
constexpr uint32_t WINDOW_WIDTH = 1152;
constexpr uint32_t WINDOW_HEIGHT = 648;
constexpr uint32_t LOGICAL_RESOLUTION_WIDTH = 576;
constexpr uint32_t LOGICAL_RESOLUTION_HEIGHT = 324;
constexpr uint32_t PHYSICS_TICKS_PER_SECOND = 60;
constexpr bool PHYSICS_INTERPOLATION = true;

Entity* spawn_player_entity(App& app) {
    Entity* entity = app.get_entity_spawner()->spawn_entity();
    entity->set_is_ticking(true);

    TransformComponent* transform_component = entity->add_component<TransformComponent>();
    transform_component->set_position(Vector2(50.0f, 50.0f));

    ImageComponent* image_component = entity->add_component<ImageComponent>();
    const std::filesystem::path path =
        PathUtils::get_assets_root_path() / "images" / "entities" / "player" / "idle" / "00.png";
    const TextureId texture_id = app.get_assets()->load_texture(path.c_str());
    image_component->set_texture_id(texture_id);

    entity->add_component<InputComponent>();
    entity->add_component<PlayerComponent>();

    return entity;
}

int main(int argc, char* argv[]) {
    App app(
        TARGET_FPS,
        VSYNC_ENABLED,
        WINDOW_TITLE,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        LOGICAL_RESOLUTION_WIDTH,
        LOGICAL_RESOLUTION_HEIGHT,
        PHYSICS_TICKS_PER_SECOND,
        PHYSICS_INTERPOLATION);

    if (!app.is_initialized()) {
        return 1;
    }

    spawn_player_entity(app);

    app.run();

    return 0;
}
