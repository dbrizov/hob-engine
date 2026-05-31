#include "renderer.h"

#include <array>
#include <format>
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

#include "engine_config.h"
#include "console.h"
#include "logging.h"
#include "sdl_context.h"

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

    // TextureRef
    TextureRef::TextureRef(Renderer& renderer, TextureId id, uint32_t width, uint32_t height)
        : m_renderer(&renderer)
        , m_id(id)
        , m_width(width)
        , m_height(height) {
    }

    TextureRef::~TextureRef() {
        reset();
    }

    TextureRef::TextureRef(TextureRef&& other) noexcept
        : m_renderer(other.m_renderer)
        , m_id(other.m_id)
        , m_width(other.m_width)
        , m_height(other.m_height) {
        other.m_renderer = nullptr;
        other.m_id = INVALID_TEXTURE_ID;
        other.m_width = 0;
        other.m_height = 0;
    }

    TextureRef& TextureRef::operator=(TextureRef&& other) noexcept {
        if (this != &other) {
            reset();
            m_renderer = other.m_renderer;
            m_id = other.m_id;
            m_width = other.m_width;
            m_height = other.m_height;

            other.m_renderer = nullptr;
            other.m_id = INVALID_TEXTURE_ID;
            other.m_width = 0;
            other.m_height = 0;
        }

        return *this;
    }

    void TextureRef::reset() {
        if (m_renderer != nullptr) {
            m_renderer->unload_texture(m_id);
            m_renderer = nullptr;
            m_id = INVALID_TEXTURE_ID;
            m_width = 0;
            m_height = 0;
        }
    }

    bool TextureRef::is_valid() const {
        return m_renderer != nullptr;
    }

    TextureId TextureRef::get_id() const {
        return m_id;
    }

    uint32_t TextureRef::get_width() const {
        return m_width;
    }

    uint32_t TextureRef::get_height() const {
        return m_height;
    }

    // Color
    Color::Color()
        : Color(0.0f, 0.0f, 0.0f, 0.0f) {
    }

    Color::Color(float r_, float g_, float b_, float a_)
        : r(r_)
        , g(g_)
        , b(b_)
        , a(a_) {
    }

    Color Color::black() { return Color(0.0f, 0.0f, 0.0f); }
    Color Color::white() { return Color(1.0f, 1.0f, 1.0f); }
    Color Color::gray() { return Color(0.5f, 0.5f, 0.5f); }
    Color Color::red() { return Color(1.0f, 0.0f, 0.0f); }
    Color Color::green() { return Color(0.0f, 1.0f, 0.0f); }
    Color Color::blue() { return Color(0.0f, 0.0f, 1.0f); }
    Color Color::yellow() { return Color(1.0f, 1.0f, 0.0f); }
    Color Color::magenta() { return Color(1.0f, 0.0f, 1.0f); }
    Color Color::cyan() { return Color(0.0f, 1.0f, 1.0f); }
    Color Color::orange() { return Color(1.0f, 0.647f, 0.0f); }

    std::string Color::to_string() const {
        return std::format("({}, {}, {}, {})", r, g, b, a);
    }

    // Renderer
    Renderer::Renderer(const EngineConfig& config, const SdlContext& sdl_context, Console& console)
        : m_sdl_context(sdl_context)
        , m_logical_width(config.graphics_config.logical_resolution_width)
        , m_logical_height(config.graphics_config.logical_resolution_height)
        , m_pixels_per_meter(config.graphics_config.pixels_per_meter)
        , m_projection(ortho_top_left(static_cast<float>(m_logical_width), static_cast<float>(m_logical_height))) {

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

        register_cvars(console);

        m_is_initialized = true;
        debug::log("Renderer initialized ({}x{} FBO)", m_logical_width, m_logical_height);
    }

    Renderer::~Renderer() {
        unload_all_textures();

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

    float Renderer::get_logical_width_f() const {
        return static_cast<float>(get_logical_width());
    }

    float Renderer::get_logical_height_f() const {
        return static_cast<float>(get_logical_height());
    }

    uint32_t Renderer::get_pixels_per_meter() const {
        return m_pixels_per_meter;
    }

    float Renderer::get_pixels_per_meter_f() const {
        return static_cast<float>(get_pixels_per_meter());
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
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA8,
                     static_cast<GLsizei>(get_logical_width()),
                     static_cast<GLsizei>(get_logical_height()),
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     nullptr);
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
        glViewport(0, 0, static_cast<GLsizei>(get_logical_width()), static_cast<GLsizei>(get_logical_height()));
        glClearColor(0.17f, 0.18f, 0.47f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer::frame_end() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSizeInPixels(m_sdl_context.get_window(), &window_w, &window_h);
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

    void Renderer::draw_sprite(TextureId texture_id,
                               const Vector2& screen_pos,
                               const Vector2& size_pixels,
                               const Vector2& pivot_pixel,
                               float rotation_rad,
                               const Color& tint) {
        glUseProgram(m_sprite_program);
        glUniformMatrix4fv(m_u_sprite_projection, 1, GL_FALSE, m_projection.data());
        glUniform2f(m_u_sprite_screen_pos, screen_pos.x, screen_pos.y);
        glUniform2f(m_u_sprite_size, size_pixels.x, size_pixels.y);
        glUniform2f(m_u_sprite_pivot_pixel, pivot_pixel.x, pivot_pixel.y);

        // World space is y-up; screen projection is y-down. Negate so positive world rotation
        // remains visually counter-clockwise.
        glUniform1f(m_u_sprite_rotation, -rotation_rad);

        glUniform4f(m_u_sprite_tint, tint.r, tint.g, tint.b, tint.a);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glBindVertexArray(m_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Renderer::draw_line(const Vector2& a, const Vector2& b, const Color& color, float thickness) {
        const float verts[] = {
            a.x, a.y, color.r, color.g, color.b, color.a,
            b.x, b.y, color.r, color.g, color.b, color.a,
        };

        glUseProgram(m_line_program);
        glUniformMatrix4fv(m_u_line_projection, 1, GL_FALSE, m_projection.data());

        glBindVertexArray(m_line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

        glLineWidth(thickness < 1.0f ? 1.0f : thickness);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
    }

    TextureId Renderer::create_texture_from_pixels(const void* rgba_pixels, int width, int height) {
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

    void Renderer::destroy_texture(TextureId id) {
        if (id != 0 && id != INVALID_TEXTURE_ID) {
            glDeleteTextures(1, &id);
        }
    }

    TextureRef Renderer::load_texture(const std::filesystem::path& full_path) {
        std::error_code error_code;
        std::filesystem::path canonical = std::filesystem::weakly_canonical(full_path, error_code);
        if (error_code) {
            canonical = full_path.lexically_normal();
        }
        const std::string key = canonical.string();

        auto it = m_path_to_id.find(key);
        if (it != m_path_to_id.end()) {
            TextureEntry& entry = m_textures.at(it->second);
            entry.ref_count += 1;

            if (m_cvar_log_textures) {
                debug::log("Renderer::load_texture cache hit: '{}' (id={}, rc={})", key, it->second, entry.ref_count);
            }

            return TextureRef(*this, it->second, entry.width, entry.height);
        }

        SDL_Surface* surface = IMG_Load(full_path.string().c_str());
        if (!surface) {
            debug::log_error("IMG_Load failed: {}", SDL_GetError());
            return TextureRef();
        }

        SDL_Surface* rgba = surface;
        if (surface->format != SDL_PIXELFORMAT_RGBA32) {
            rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(surface);
            if (!rgba) {
                debug::log_error("SDL_ConvertSurface failed: {}", SDL_GetError());
                return TextureRef();
            }
        }

        const TextureId texture_id = create_texture_from_pixels(rgba->pixels, rgba->w, rgba->h);
        const uint32_t w = static_cast<uint32_t>(rgba->w);
        const uint32_t h = static_cast<uint32_t>(rgba->h);
        const int ref_count = 1;
        SDL_DestroySurface(rgba);

        m_textures.emplace(texture_id, TextureEntry(w, h, key, ref_count));
        m_path_to_id.emplace(key, texture_id);

        if (m_cvar_log_textures) {
            debug::log("Renderer::load_texture loaded: '{}' (id={}, rc={})", key, texture_id, ref_count);
        }

        return TextureRef(*this, texture_id, w, h);
    }

    bool Renderer::unload_texture(TextureId id) {
        auto it = m_textures.find(id);
        if (it == m_textures.end()) {
            debug::log_error("Renderer::unload_texture: 'unknown' (id={})", id);
            return false;
        }

        TextureEntry& entry = it->second;
        entry.ref_count -= 1;

        if (entry.ref_count > 0) {
            if (m_cvar_log_textures) {
                debug::log("Renderer::unload_texture: '{}' (id={}, rc={})", entry.path, id, entry.ref_count);
            }

            return true;
        }

        if (m_cvar_log_textures) {
            debug::log("Renderer::unload_texture: '{}' (id={}, rc={}) [destroyed]", entry.path, id, entry.ref_count);
        }

        destroy_texture(id);
        m_path_to_id.erase(entry.path);
        m_textures.erase(it);
        return true;
    }

    void Renderer::unload_all_textures() {
        for (auto& [id, entry] : m_textures) {
            debug::log_error("Renderer: leaked texture '{}' (id={}, rc={})", entry.path, id, entry.ref_count);
            destroy_texture(id);
        }

        m_textures.clear();
        m_path_to_id.clear();
    }

    void Renderer::register_cvars(Console& console) {
        console.register_cvar("rend_log_textures",
                              "Log every texture load/unload/cache-hit",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_log_textures = cvar.bool_value();
                              });
    }
}
