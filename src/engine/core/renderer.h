#pragma once

#include <cstdint>
#include <string>

#include "engine/math/vector2.h"

struct SDL_Window;

namespace hob {
    struct GraphicsConfig;
    using GlTexture = uint32_t;

    constexpr const char* GLSL_VERSION = "#version 330 core\n";

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        Color();
        Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255);

        static Color black();
        static Color white();
        static Color gray();
        static Color red();
        static Color green();
        static Color blue();
        static Color yellow();
        static Color magenta();
        static Color cyan();
        static Color orange();

        std::string to_string() const;
    };

    class Renderer {
        SDL_Window* m_window;
        uint32_t m_logical_width;
        uint32_t m_logical_height;

        // Offscreen FBO at logical resolution.
        uint32_t m_fbo = 0;
        uint32_t m_fbo_color_texture = 0;

        // Sprite pipeline.
        uint32_t m_sprite_program = 0;
        int32_t m_u_sprite_projection = -1;
        int32_t m_u_sprite_screen_pos = -1;
        int32_t m_u_sprite_size = -1;
        int32_t m_u_sprite_pivot_pixel = -1;
        int32_t m_u_sprite_rotation = -1;
        int32_t m_u_sprite_tint = -1;
        uint32_t m_quad_vao = 0;
        uint32_t m_quad_vbo = 0;

        // Solid-color pipeline (lines).
        uint32_t m_line_program = 0;
        int32_t m_u_line_projection = -1;
        uint32_t m_line_vao = 0;
        uint32_t m_line_vbo = 0;

        // Fullscreen blit pipeline (FBO -> window).
        uint32_t m_blit_program = 0;
        uint32_t m_blit_vao = 0;
        uint32_t m_blit_vbo = 0;

        bool m_is_initialized = false;

    public:
        Renderer(SDL_Window* window, const GraphicsConfig& graphics_config);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool is_initialized() const;

        uint32_t get_logical_width() const;
        uint32_t get_logical_height() const;

        // Begin a frame: bind FBO, set logical viewport, clear.
        void frame_start();

        // End a frame: unbind FBO, blit to window back-buffer.
        // After this, ImGui can render to the default framebuffer.
        void frame_end();

        // Sprite draw in logical screen space (top-left origin, y-down).
        // screen_pos is the unrotated top-left of the rect.
        // pivot_pixel is the rotation pivot in pixel coords relative to that top-left.
        // rotation_rad is in world-space radians (CCW in y-up).
        void draw_sprite(GlTexture texture,
                         Vector2 screen_pos,
                         Vector2 size,
                         Vector2 pivot_pixel,
                         float rotation_rad,
                         Color tint);

        // Line draw in logical screen space.
        void draw_line(Vector2 a, Vector2 b, float thickness, Color color);

        // Texture upload helpers used by Assets.
        static GlTexture create_texture_from_pixels(const void* rgba_pixels, int width, int height);
        static void destroy_texture(GlTexture texture);
        static void get_texture_size(GlTexture texture, int& out_width, int& out_height);

    private:
        bool init_sprite_pipeline();
        bool init_line_pipeline();
        bool init_blit_pipeline();
        bool init_fbo();
    };
}
