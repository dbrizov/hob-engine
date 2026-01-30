#include "SdlContext.h"

#include <SDL.h>
#include <SDL_image.h>
#include <fmt/base.h>

SdlContext::SdlContext(uint32_t screen_width, uint32_t screen_height, const std::string& window_title)
    : m_is_initialized(false)
    , m_window(nullptr)
    , m_renderer(nullptr)
    , m_screen_width(screen_width)
    , m_screen_height(screen_height)
    , m_window_title(window_title) {
    // SDL_Init
    int sld_init_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS;
    if (SDL_Init(sld_init_flags) != 0) {
        fmt::println(stderr, "SDL_Init Error: {}", SDL_GetError());
        return;
    }

    fmt::println("SDL_Init");

    // IMG_Init
    int img_init_flags = IMG_INIT_PNG;
    if ((IMG_Init(img_init_flags) & img_init_flags) != img_init_flags) {
        fmt::println(stderr, "IMG_Init Error: {}", IMG_GetError());
        SDL_Quit();
        return;
    }

    fmt::println("IMG_Init");

    // SDL_CreateWindow
    m_window = SDL_CreateWindow(
        m_window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_screen_width),
        static_cast<int>(m_screen_height),
        SDL_WINDOW_SHOWN);

    if (!m_window) {
        fmt::println(stderr, "SDL_CreateWindow Error: {}", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return;
    }

    fmt::println("SDL_CreateWindow");

    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        fmt::println(stderr, "SDL_CreateRenderer Error: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
        IMG_Quit();
        SDL_Quit();
        return;
    }

    fmt::println("SDL_CreateRenderer");

    m_is_initialized = true;
}

SdlContext::~SdlContext() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        fmt::println("SDL_DestroyRenderer");
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        fmt::println("SDL_DestroyWindow");
    }

    IMG_Quit();
    fmt::println("IMG_Quit");

    SDL_Quit();
    fmt::println("SDL_Quit");
}

bool SdlContext::is_initialized() const {
    return m_is_initialized;
}

SDL_Window* SdlContext::get_window() const {
    return m_window;
}

SDL_Renderer* SdlContext::get_renderer() const {
    return m_renderer;
}

uint32_t SdlContext::get_screen_width() const {
    return m_screen_width;
}

uint32_t SdlContext::get_screen_height() const {
    return m_screen_height;
}
