#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

struct ImGuiContext;

namespace hob {
    class SdlContext;

    class ImGuiSystem {
        bool m_is_initialized = false;
        ImGuiContext* m_context = nullptr;
        SDL_GPUDevice* m_gpu_device = nullptr;

    public:
        explicit ImGuiSystem(const SdlContext& sdl_context);
        ~ImGuiSystem();

        ImGuiSystem(const ImGuiSystem&) = delete;
        ImGuiSystem& operator=(const ImGuiSystem&) = delete;

        ImGuiSystem(ImGuiSystem&&) = delete;
        ImGuiSystem& operator=(ImGuiSystem&&) = delete;

        bool is_initialized() const;

        void process_event(const SDL_Event& event);

        // Open the ImGui frame. Must run before any tick/gameplay code calls ImGui widgets.
        void new_frame();

        // Finalizes the ImGui frame, uploads draw data via a copy pass on `cmd`, then opens
        // a render pass on `swap_tex` (LOAD_OP_LOAD so prior contents are preserved) and
        // records ImGui draw commands into it. Pairs with new_frame as an alternative to
        // discard_frame — exactly one of the two must run per new_frame.
        void render_pass(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swap_tex);

        // Discard the current ImGui frame without producing draw data (e.g. when the
        // swapchain texture could not be acquired). Pairs with new_frame as an alternative
        // to record_draw_data_pass — exactly one of the two must run per new_frame.
        void discard_frame();
    };
}
