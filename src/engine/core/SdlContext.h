#ifndef HOB_ENGINE_SDLCONTEXT_H
#define HOB_ENGINE_SDLCONTEXT_H


struct SDL_Window;
struct SDL_Renderer;

namespace hob {
    struct GraphicsConfig;

    class SdlContext {
        bool m_is_initialized = false;
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;

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

        void frame_start();
        void frame_end();
    };
}


#endif //HOB_ENGINE_SDLCONTEXT_H
