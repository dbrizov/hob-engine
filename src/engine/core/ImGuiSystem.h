#ifndef HOB_ENGINE_IMGUISYSTEM_H
#define HOB_ENGINE_IMGUISYSTEM_H
#include <SDL_events.h>


struct ImGuiContext;
struct SDL_Window;
struct SDL_Renderer;


class ImGuiSystem {
    bool m_is_initialized = false;
    ImGuiContext* m_context = nullptr;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

public:
    ImGuiSystem(SDL_Window* window, SDL_Renderer* renderer);
    ~ImGuiSystem();

    ImGuiSystem(const ImGuiSystem&) = delete;
    ImGuiSystem& operator=(const ImGuiSystem&) = delete;

    ImGuiSystem(ImGuiSystem&&) = delete;
    ImGuiSystem& operator=(ImGuiSystem&&) = delete;

    bool is_initialized() const;

    void process_event(const SDL_Event& event);

    void frame_start();
    void frame_end();
};


#endif //HOB_ENGINE_IMGUISYSTEM_H
