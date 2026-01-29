#include <filesystem>
#include <fmt/base.h>

#include "engine/App.h"

constexpr uint32_t FPS = 60;
constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;
const char* const WINDOW_TITLE = "SDL2 Window";

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

int main(int argc, char* argv[]) {
    std::string input_config_path = get_input_config_path().string();
    fmt::println("input_config_path: '{}'", input_config_path);

    AppConfig config;
    config
        .set_fps(FPS)
        .set_screen_width(SCREEN_WIDTH)
        .set_screen_height(SCREEN_HEIGHT)
        .set_window_title(WINDOW_TITLE)
        .set_input_config_path(input_config_path);

    App app = App(config);
    if (app.init()) {
        app.run();
    }

    return 0;
}
