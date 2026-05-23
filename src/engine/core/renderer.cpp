#include "renderer.h"

#include <array>
#include <format>
#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "app.h"
#include "logging.h"

namespace hob {
    Color::Color()
        : Color(0, 0, 0, 0) {
    }

    Color::Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
        : r(r_)
        , g(g_)
        , b(b_)
        , a(a_) {
    }

    Color Color::black() { return Color(0, 0, 0); }
    Color Color::white() { return Color(255, 255, 255); }
    Color Color::gray() { return Color(128, 128, 128); }
    Color Color::red() { return Color(255, 0, 0); }
    Color Color::green() { return Color(0, 255, 0); }
    Color Color::blue() { return Color(0, 0, 255); }
    Color Color::yellow() { return Color(255, 255, 0); }
    Color Color::magenta() { return Color(255, 0, 255); }
    Color Color::cyan() { return Color(0, 255, 255); }
    Color Color::orange() { return Color(255, 165, 0); }

    std::string Color::to_string() const {
        return std::format("({}, {}, {}, {})", r, g, b, a);
    }
}

namespace hob {
    namespace {
        const char* SPRITE_VS = R"(
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_projection;
uniform vec2 u_screen_pos;
uniform vec2 u_size;
uniform vec2 u_pivot_pixel;
uniform float u_rotation;

out vec2 v_uv;

void main() {
    vec2 p = a_pos * u_size;
    vec2 d = p - u_pivot_pixel;
    float c = cos(u_rotation);
    float s = sin(u_rotation);
    vec2 r = vec2(c * d.x - s * d.y, s * d.x + c * d.y);
    vec2 screen = u_screen_pos + u_pivot_pixel + r;
    gl_Position = u_projection * vec4(screen, 0.0, 1.0);
    v_uv = a_uv;
}
)";

        const char* SPRITE_FS = R"(
in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_texture;
uniform vec4 u_tint;

void main() {
    frag_color = texture(u_texture, v_uv) * u_tint;
}
)";

        const char* LINE_VS = R"(
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec4 a_color;

uniform mat4 u_projection;

out vec4 v_color;

void main() {
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
    v_color = a_color;
}
)";

        const char* LINE_FS = R"(
in vec4 v_color;
out vec4 frag_color;

void main() {
    frag_color = v_color;
}
)";

        const char* BLIT_VS = R"(
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 v_uv;

void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);
    v_uv = a_uv;
}
)";

        const char* BLIT_FS = R"(
in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_texture;

void main() {
    frag_color = texture(u_texture, v_uv);
}
)";

        bool compile_shader(GLenum type, const char* source, GLuint& out_shader) {
            GLuint shader = glCreateShader(type);
            const char* sources[] = {GLSL_VERSION, source};
            glShaderSource(shader, 2, sources, nullptr);
            glCompileShader(shader);

            GLint status = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                char log[1024] = {};
                glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
                debug::log_error("Shader compile failed: {}", log);
                glDeleteShader(shader);
                return false;
            }

            out_shader = shader;
            return true;
        }

        bool link_program(const char* vs_source, const char* fs_source, GLuint& out_program) {
            GLuint vs = 0;
            GLuint fs = 0;
            if (!compile_shader(GL_VERTEX_SHADER, vs_source, vs)) {
                return false;
            }

            if (!compile_shader(GL_FRAGMENT_SHADER, fs_source, fs)) {
                glDeleteShader(vs);
                return false;
            }

            GLuint program = glCreateProgram();
            glAttachShader(program, vs);
            glAttachShader(program, fs);
            glLinkProgram(program);

            GLint status = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &status);
            glDeleteShader(vs);
            glDeleteShader(fs);

            if (status != GL_TRUE) {
                char log[1024] = {};
                glGetProgramInfoLog(program, sizeof(log), nullptr, log);
                debug::log_error("Program link failed: {}", log);
                glDeleteProgram(program);
                return false;
            }

            out_program = program;
            return true;
        }

        // Top-left origin ortho: x in [0, w], y in [0, h] mapping to clip space.
        // Y is flipped: y=0 -> top of screen (clip y = +1), y=h -> bottom (clip y = -1).
        std::array<float, 16> ortho_top_left(float w, float h) {
            std::array<float, 16> m{};
            m[0] = 2.0f / w;
            m[5] = -2.0f / h;
            m[10] = -1.0f;
            m[12] = -1.0f;
            m[13] = 1.0f;
            m[15] = 1.0f;
            return m;
        }
    }

    Renderer::Renderer(SDL_Window* window, const GraphicsConfig& graphics_config)
        : m_window(window)
        , m_logical_width(graphics_config.logical_resolution_width)
        , m_logical_height(graphics_config.logical_resolution_height) {

        if (!init_sprite_pipeline()) {
            return;
        }
        if (!init_line_pipeline()) {
            return;
        }
        if (!init_blit_pipeline()) {
            return;
        }
        if (!init_fbo()) {
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        m_is_initialized = true;
        debug::log("Renderer initialized ({}x{} FBO)", m_logical_width, m_logical_height);
    }

    Renderer::~Renderer() {
        if (m_quad_vbo)
            glDeleteBuffers(1, &m_quad_vbo);
        if (m_quad_vao)
            glDeleteVertexArrays(1, &m_quad_vao);
        if (m_sprite_program)
            glDeleteProgram(m_sprite_program);

        if (m_line_vbo)
            glDeleteBuffers(1, &m_line_vbo);
        if (m_line_vao)
            glDeleteVertexArrays(1, &m_line_vao);
        if (m_line_program)
            glDeleteProgram(m_line_program);

        if (m_blit_vbo)
            glDeleteBuffers(1, &m_blit_vbo);
        if (m_blit_vao)
            glDeleteVertexArrays(1, &m_blit_vao);
        if (m_blit_program)
            glDeleteProgram(m_blit_program);

        if (m_fbo_color_texture)
            glDeleteTextures(1, &m_fbo_color_texture);
        if (m_fbo)
            glDeleteFramebuffers(1, &m_fbo);

        debug::log("Renderer uninitialized ({}x{} FBO)", m_logical_width, m_logical_height);
    }

    bool Renderer::is_initialized() const {
        return m_is_initialized;
    }

    uint32_t Renderer::get_logical_width() const {
        return m_logical_width;
    }

    uint32_t Renderer::get_logical_height() const {
        return m_logical_height;
    }

    bool Renderer::init_sprite_pipeline() {
        if (!link_program(SPRITE_VS, SPRITE_FS, m_sprite_program)) {
            return false;
        }

        m_u_sprite_projection = glGetUniformLocation(m_sprite_program, "u_projection");
        m_u_sprite_screen_pos = glGetUniformLocation(m_sprite_program, "u_screen_pos");
        m_u_sprite_size = glGetUniformLocation(m_sprite_program, "u_size");
        m_u_sprite_pivot_pixel = glGetUniformLocation(m_sprite_program, "u_pivot_pixel");
        m_u_sprite_rotation = glGetUniformLocation(m_sprite_program, "u_rotation");
        m_u_sprite_tint = glGetUniformLocation(m_sprite_program, "u_tint");

        // Unit quad. Identity UVs: quad-local (0,0) is the top-left in screen space (because
        // the projection maps screen y=0 to clip y=+1), and SDL_image-loaded pixels have row 0
        // at the visual top, which OpenGL stores at texel y=0. So the top vertex samples UV y=0.
        const float verts[] = {
            // pos        uv
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
        };

        glGenVertexArrays(1, &m_quad_vao);
        glGenBuffers(1, &m_quad_vbo);
        glBindVertexArray(m_quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glBindVertexArray(0);

        return true;
    }

    bool Renderer::init_line_pipeline() {
        if (!link_program(LINE_VS, LINE_FS, m_line_program)) {
            return false;
        }

        m_u_line_projection = glGetUniformLocation(m_line_program, "u_projection");

        glGenVertexArrays(1, &m_line_vao);
        glGenBuffers(1, &m_line_vbo);
        glBindVertexArray(m_line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
        // Reserve room for two vertices: pos(2f) + color(4f) = 6f * 2 = 48 bytes.
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glBindVertexArray(0);

        return true;
    }

    bool Renderer::init_blit_pipeline() {
        if (!link_program(BLIT_VS, BLIT_FS, m_blit_program)) {
            return false;
        }

        // Fullscreen triangle pair in NDC; UVs flipped so the FBO's y-up texture appears
        // with y-down screen orientation matching what was rendered into it.
        const float verts[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
        };

        glGenVertexArrays(1, &m_blit_vao);
        glGenBuffers(1, &m_blit_vbo);
        glBindVertexArray(m_blit_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_blit_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glBindVertexArray(0);

        return true;
    }

    bool Renderer::init_fbo() {
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        glGenTextures(1, &m_fbo_color_texture);
        glBindTexture(GL_TEXTURE_2D, m_fbo_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     static_cast<GLsizei>(m_logical_width),
                     static_cast<GLsizei>(m_logical_height),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_color_texture, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            debug::log_error("FBO incomplete: 0x{:x}", static_cast<unsigned int>(status));
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    void Renderer::frame_start() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, static_cast<GLsizei>(m_logical_width), static_cast<GLsizei>(m_logical_height));
        glClearColor(43.0f / 255.0f, 47.0f / 255.0f, 119.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::frame_end() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSizeInPixels(m_window, &window_w, &window_h);
        glViewport(0, 0, window_w, window_h);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_blit_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_fbo_color_texture);
        glBindVertexArray(m_blit_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Renderer::draw_sprite(GlTexture texture,
                               Vector2 screen_pos,
                               Vector2 size,
                               Vector2 pivot_pixel,
                               float rotation_rad,
                               Color tint) {
        const std::array<float, 16> projection =
            ortho_top_left(static_cast<float>(m_logical_width), static_cast<float>(m_logical_height));

        glUseProgram(m_sprite_program);
        glUniformMatrix4fv(m_u_sprite_projection, 1, GL_FALSE, projection.data());
        glUniform2f(m_u_sprite_screen_pos, screen_pos.x, screen_pos.y);
        glUniform2f(m_u_sprite_size, size.x, size.y);
        glUniform2f(m_u_sprite_pivot_pixel, pivot_pixel.x, pivot_pixel.y);

        // World space is y-up; screen projection is y-down. Negate so positive world rotation
        // remains visually counter-clockwise.
        glUniform1f(m_u_sprite_rotation, -rotation_rad);

        glUniform4f(m_u_sprite_tint,
                    tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f, tint.a / 255.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(m_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Renderer::draw_line(Vector2 a, Vector2 b, float thickness, Color color) {
        const std::array<float, 16> projection =
            ortho_top_left(static_cast<float>(m_logical_width), static_cast<float>(m_logical_height));

        const float r = color.r / 255.0f;
        const float g = color.g / 255.0f;
        const float bl = color.b / 255.0f;
        const float al = color.a / 255.0f;

        const float verts[] = {
            a.x, a.y, r, g, bl, al,
            b.x, b.y, r, g, bl, al,
        };

        glUseProgram(m_line_program);
        glUniformMatrix4fv(m_u_line_projection, 1, GL_FALSE, projection.data());

        glBindVertexArray(m_line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

        glLineWidth(thickness < 1.0f ? 1.0f : thickness);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
    }

    GlTexture Renderer::create_texture_from_pixels(const void* rgba_pixels, int width, int height) {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return texture;
    }

    void Renderer::destroy_texture(GlTexture texture) {
        if (texture != 0) {
            GLuint id = texture;
            glDeleteTextures(1, &id);
        }
    }

    void Renderer::get_texture_size(GlTexture texture, int& out_width, int& out_height) {
        GLint w = 0;
        GLint h = 0;
        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        out_width = w;
        out_height = h;
    }
}
