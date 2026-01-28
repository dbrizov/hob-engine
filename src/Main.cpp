#ifdef NDEBUG // If in release mode

#ifdef _WIN32 // If on Windows
#include <windows.h>
#else // In on Linux/Unix
#include <unistd.h>
#include <limits.h>
#endif

#endif


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
    // Return the root directory of Main.cpp
    std::filesystem::path source_file = __FILE__;
    std::filesystem::path project_root = source_file.parent_path().parent_path();
    return project_root;
#else
    // (IN RELEASE MODE)
    // Return the root directory of the executable
#ifdef _WIN32
    // We are on Windows
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::filesystem::path executable_root = std::filesystem::path(path).parent_path();
    return executable_root;
#else
    // We are on Linux/Unix
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::filesystem::path executable_root = std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
    return executable_root;
#endif
#endif
}

std::filesystem::path get_input_config_path() {
    std::filesystem::path root_path = get_root_path();
    std::filesystem::path input_config_path = root_path / "engine_config" / "input_config.json";
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
