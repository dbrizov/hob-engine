#ifndef HOB_ENGINE_SDLCONTEXT_H
#define HOB_ENGINE_SDLCONTEXT_H


struct GraphicsConfig;
struct SDL_Window;
struct SDL_Renderer;


class SdlContext {
    bool m_is_initialized;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

public:
    explicit SdlContext(const GraphicsConfig& graphics_config);
    ~SdlContext();

    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;

    SdlContext(SdlContext&&) = delete;
    SdlContext& operator=(SdlContext&&) = delete;

    bool is_initialized() const;
    SDL_Window* get_window() const;
    SDL_Renderer* get_renderer() const;
};


#endif //HOB_ENGINE_SDLCONTEXT_H
