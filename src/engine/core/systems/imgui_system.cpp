#include "imgui_system.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "engine/core/logging.h"
#include "sdl_context.h"

namespace hob {
    ImGuiSystem::ImGuiSystem(const SdlContext& sdl_context) {
        SDL_Window* window = sdl_context.get_window();
        m_gpu_device = sdl_context.get_gpu_device();

        if (!window || !m_gpu_device) {
            debug::log_error("ImGuiSystem init failed: window/GPU device is null");
            return;
        }

        IMGUI_CHECKVERSION();

        m_context = ImGui::CreateContext();
        if (!m_context) {
            debug::log_error("ImGui_CreateContext failed");
            return;
        }

        debug::log("ImGui_CreateContext()");

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplSDL3_InitForSDLGPU(window)) {
            debug::log_error("ImGui_ImplSDL3_InitForSDLGPU failed");
            ImGui::DestroyContext(m_context);
            m_context = nullptr;
            return;
        }

        debug::log("ImGui_ImplSDL3_InitForSDLGPU");

        ImGui_ImplSDLGPU3_InitInfo init_info{};
        init_info.Device = m_gpu_device;
        init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(m_gpu_device, window);
        init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

        if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
            debug::log_error("ImGui_ImplSDLGPU3_Init failed");
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext(m_context);
            m_context = nullptr;
            return;
        }

        debug::log("ImGui_ImplSDLGPU3_Init");

        m_is_initialized = true;
    }

    ImGuiSystem::~ImGuiSystem() {
        if (!m_is_initialized) {
            return;
        }

        ImGui_ImplSDLGPU3_Shutdown();
        debug::log("ImGui_ImplSDLGPU3_Shutdown");

        ImGui_ImplSDL3_Shutdown();
        debug::log("ImGui_ImplSDL3_Shutdown");

        ImGui::DestroyContext(m_context);
        m_context = nullptr;
        debug::log("ImGui_DestroyContext");
    }

    bool ImGuiSystem::is_initialized() const {
        return m_is_initialized;
    }

    void ImGuiSystem::process_event(const SDL_Event& event) {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void ImGuiSystem::new_frame() {
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::prepare_draw_data(SDL_GPUCommandBuffer* cmd) {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, cmd);
    }

    void ImGuiSystem::record_draw_data(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* cmd) {
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmd, pass);
    }

    void ImGuiSystem::discard_frame() {
        ImGui::EndFrame();
    }
}
