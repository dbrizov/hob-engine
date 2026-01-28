#ifndef CPP_PLATFORMER_APP_H
#define CPP_PLATFORMER_APP_H

struct SDL_Window;
struct SDL_Renderer;


class App {
private:
    int m_fps;
    int m_screen_width;
    int m_screen_height;
    const char* m_window_title;
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_renderer;

public:
    App(int fps, int screen_width, int screen_height, const char* window_title);
    ~App();

    bool init();
    void run();
};


#endif //CPP_PLATFORMER_APP_H
