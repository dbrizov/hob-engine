#include <filesystem>
#include <fmt/base.h>

#include "engine/components/ImageComponent.h"
#include "engine/components/InputComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"
#include "engine/entity/Entity.h"

constexpr uint32_t FPS = 60;
constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;
const std::string WINDOW_TITLE = "SDL2 Window";

std::filesystem::path get_root_path() {
#ifndef NDEBUG
    // (IN DEBUG MODE)
    // Return the root directory of the project
    std::filesystem::path source_file_path = __FILE__;
    std::filesystem::path project_root_path = source_file_path.parent_path().parent_path();
    return project_root_path;
#else
    // (IN RELEASE MODE)
    // Return the current directory of the executable
    std::filesystem::path current_path = std::filesystem::current_path();
    return current_path;
#endif
}

std::filesystem::path get_input_config_path() {
    std::filesystem::path root_path = get_root_path();
    std::filesystem::path input_config_path = root_path / "config" / "input_config.json";
    return input_config_path;
}

std::filesystem::path get_assets_root_path() {
    std::filesystem::path root_path = get_root_path();
    std::filesystem::path assets_root_path = root_path / "assets";
    return assets_root_path;
}

int main(int argc, char* argv[]) {
    std::filesystem::path input_config_path = get_input_config_path();
    fmt::println("input_config_path: '{}'", input_config_path.string());

    std::filesystem::path assets_root_path = get_assets_root_path();
    fmt::println("assets_root_path: '{}'", assets_root_path.string());

    App app(
        FPS,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        WINDOW_TITLE,
        input_config_path,
        assets_root_path);

    if (!app.is_initialized()) {
        return 1;
    }

    Entity* entity = app.get_entity_spawner()->spawn_entity();
    TransformComponent* transform_component = entity->add_component<TransformComponent>();
    transform_component->set_position(Vector2(50.0f, 50.0f));
    transform_component->set_scale(Vector2(2.0f, 2.0f));

    ImageComponent* image_component = entity->add_component<ImageComponent>();
    const std::filesystem::path path = app.get_assets()->get_assets_root_path() / "images" / "entities" / "player" / "idle" / "00.png";
    const TextureId texture_id = app.get_assets()->load_texture(path.c_str());
    image_component->set_texture_id(texture_id);

    InputComponent* input_component = entity->add_component<InputComponent>();
    input_component->bind_axis("horizontal", [](float axis) {
        fmt::println("horizontal: {}", axis);
    });

    input_component->bind_axis("vertical", [](float axis) {
        fmt::println("vertical: {}", axis);
    });

    input_component->bind_action("submit", InputEventType::PRESSED, []() {
        fmt::println("submit_pressed");
    });

    input_component->bind_action("submit", InputEventType::RELEASED, []() {
        fmt::println("submit_released");
    });

    app.run();

    return 0;
}
