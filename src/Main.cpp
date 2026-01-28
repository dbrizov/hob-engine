#include "engine/App.h"

const int FPS = 60;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const char* const WINDOW_TITLE = "SDL2 Window";

int main(int argc, char* argv[]) {
    App app = App(FPS, SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    if (app.init()) {
        app.run();
    }

    return 0;
}
