#include "imgui_system.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <fmt/base.h>

namespace hob {
    ImGuiSystem::ImGuiSystem(SDL_Window* window, SDL_Renderer* renderer)
        : m_is_initialized(false)
          , m_context(nullptr)
          , m_window(window)
          , m_renderer(renderer) {
        if (!m_window || !m_renderer) {
            fmt::println(stderr, "ImGuiSystem init failed: window/renderer is null");
            return;
        }

        IMGUI_CHECKVERSION();

        // Create context
        m_context = ImGui::CreateContext();
        if (!m_context) {
            fmt::println(stderr, "ImGui_CreateContext failed");
            return;
        }

        fmt::println("ImGui_CreateContext()");

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        // Init backend for SDL
        if (!ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_renderer)) {
            fmt::println(stderr, "ImGui_ImplSDL2_InitForSDLRenderer failed");
            ImGui::DestroyContext(m_context);
            return;
        }

        fmt::println("ImGui_ImplSDL2_InitForSDLRenderer");

        // Init backend for SDL_Renderer
        if (!ImGui_ImplSDLRenderer2_Init(m_renderer)) {
            fmt::println(stderr, "ImGui_ImplSDLRenderer2_Init failed");
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext(m_context);
            return;
        }

        fmt::println("ImGui_ImplSDLRenderer2_Init");

        m_is_initialized = true;
    }

    ImGuiSystem::~ImGuiSystem() {
        if (!m_is_initialized) {
            return;
        }

        ImGui_ImplSDLRenderer2_Shutdown();
        fmt::println("ImGui_ImplSDLRenderer2_Shutdown");

        ImGui_ImplSDL2_Shutdown();
        fmt::println("ImGui_ImplSDL2_Shutdown");

        ImGui::DestroyContext(m_context);
        fmt::println("ImGui_DestroyContext");
    }

    bool ImGuiSystem::is_initialized() const {
        return m_is_initialized;
    }

    void ImGuiSystem::process_event(const SDL_Event& event) {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    void ImGuiSystem::frame_start() {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::frame_end() {
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    }
}
