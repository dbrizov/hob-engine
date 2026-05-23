#include "imgui_system.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "logging.h"
#include "renderer.h"

namespace hob {
    ImGuiSystem::ImGuiSystem(SDL_Window* window, SDL_GLContext gl_context)
        : m_window(window)
        , m_gl_context(gl_context) {
        if (!m_window || !m_gl_context) {
            debug::log_error("ImGuiSystem init failed: window/GL context is null");
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

        ImGui::StyleColorsDark();

        if (!ImGui_ImplSDL3_InitForOpenGL(m_window, m_gl_context)) {
            debug::log_error("ImGui_ImplSDL3_InitForOpenGL failed");
            ImGui::DestroyContext(m_context);
            return;
        }

        debug::log("ImGui_ImplSDL3_InitForOpenGL");

        if (!ImGui_ImplOpenGL3_Init(GLSL_VERSION)) {
            debug::log_error("ImGui_ImplOpenGL3_Init failed");
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext(m_context);
            return;
        }

        debug::log("ImGui_ImplOpenGL3_Init");

        m_is_initialized = true;
    }

    ImGuiSystem::~ImGuiSystem() {
        if (!m_is_initialized) {
            return;
        }

        ImGui_ImplOpenGL3_Shutdown();
        debug::log("ImGui_ImplOpenGL3_Shutdown");

        ImGui_ImplSDL3_Shutdown();
        debug::log("ImGui_ImplSDL3_Shutdown");

        ImGui::DestroyContext(m_context);
        debug::log("ImGui_DestroyContext");
    }

    bool ImGuiSystem::is_initialized() const {
        return m_is_initialized;
    }

    void ImGuiSystem::process_event(const SDL_Event& event) {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void ImGuiSystem::frame_start() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::frame_end() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
