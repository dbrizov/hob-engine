#include "engine/App.h"

constexpr uint32_t FPS = 60;
constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;
const char* const WINDOW_TITLE = "SDL2 Window";

int main(int argc, char* argv[]) {
    AppConfig config;
    config
        .set_fps(FPS)
        .set_screen_width(SCREEN_WIDTH)
        .set_screen_height(SCREEN_HEIGHT)
        .set_window_title(WINDOW_TITLE);

    App app = App(config);
    if (app.init()) {
        app.run();
    }

    return 0;
}
