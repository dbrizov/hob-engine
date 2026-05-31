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

        // Finalize ImGui draw data and upload its vertex/index buffers to the GPU.
        // Uses an internal copy pass, so this MUST be called outside any open render pass.
        void prepare_draw_data(SDL_GPUCommandBuffer* cmd);

        // Record ImGui draw commands into `pass`. Must be called inside an open render pass,
        // after prepare_draw_data has run on the same command buffer.
        void record_draw_data(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* cmd);

        // Discard the current ImGui frame without producing draw data (e.g. when the
        // swapchain texture could not be acquired). Pairs with new_frame as an alternative
        // to prepare_draw_data — exactly one of the two must run per new_frame.
        void discard_frame();
    };
}
