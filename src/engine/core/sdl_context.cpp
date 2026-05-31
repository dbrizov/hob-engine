#include "sdl_context.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "engine_config.h"
#include "logging.h"

namespace hob {
    SdlContext::SdlContext(const GraphicsConfig& graphics_config) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            debug::log_error("SDL_Init Error: {}", SDL_GetError());
            return;
        }

        debug::log("SDL_Init");

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

        m_window = SDL_CreateWindow(
            graphics_config.window_title.c_str(),
            static_cast<int>(graphics_config.window_width),
            static_cast<int>(graphics_config.window_height),
            SDL_WINDOW_OPENGL);

        if (!m_window) {
            debug::log_error("SDL_CreateWindow Error: {}", SDL_GetError());
            SDL_Quit();
            return;
        }

        debug::log("SDL_CreateWindow");

        m_gl_context = SDL_GL_CreateContext(m_window);
        if (!m_gl_context) {
            debug::log_error("SDL_GL_CreateContext Error: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return;
        }

        SDL_GL_MakeCurrent(m_window, m_gl_context);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            debug::log_error("gladLoadGLLoader failed");
            SDL_GL_DestroyContext(m_gl_context);
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return;
        }

        SDL_GL_SetSwapInterval(graphics_config.vsync_enabled ? 1 : 0);

        debug::log("OpenGL {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        m_is_initialized = true;
    }

    SdlContext::~SdlContext() {
        if (m_gl_context) {
            SDL_GL_DestroyContext(m_gl_context);
            debug::log("SDL_GL_DestroyContext");
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
            debug::log("SDL_DestroyWindow");
        }

        SDL_Quit();
        debug::log("SDL_Quit");
    }

    bool SdlContext::is_initialized() const {
        return m_is_initialized;
    }

    SDL_Window* SdlContext::get_window() const {
        return m_window;
    }

    SDL_GLContext SdlContext::get_gl_context() const {
        return m_gl_context;
    }

    void SdlContext::swap() {
        SDL_GL_SwapWindow(m_window);
    }
}
