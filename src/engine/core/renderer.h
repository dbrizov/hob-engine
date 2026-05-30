#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "engine/math/constants.h"
#include "engine/math/vector2.h"

namespace hob {
    struct AppConfig;
    class SdlContext;
    class Console;

    using TextureId = uint32_t;
    constexpr TextureId INVALID_TEXTURE_ID = MAX_UINT32;

    constexpr const char* GLSL_VERSION = "#version 330 core\n";

    struct Color {
        float r;
        float g;
        float b;
        float a;

        Color();
        Color(float r_, float g_, float b_, float a_ = 1.0f);

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
        struct TextureEntry {
            int width;
            int height;
            std::string path;
            int ref_count;

            TextureEntry(int _width, int _height, std::string _path, int _ref_count)
                : width(_width)
                , height(_height)
                , path(std::move(_path))
                , ref_count(_ref_count) {
            }
        };

        const SdlContext& m_sdl_context;
        uint32_t m_logical_width;
        uint32_t m_logical_height;
        uint32_t m_pixels_per_meter;
        std::array<float, 16> m_projection{};

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

        // Texture cache.
        std::unordered_map<TextureId, TextureEntry> m_textures;
        std::unordered_map<std::string, TextureId> m_path_to_id;
        bool m_cvar_log_textures = false;

        bool m_is_initialized = false;

    public:
        Renderer(const AppConfig& config, const SdlContext& sdl_context, Console& console);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool is_initialized() const;

        uint32_t get_logical_width() const;
        uint32_t get_logical_height() const;
        float get_logical_width_f() const;
        float get_logical_height_f() const;

        uint32_t get_pixels_per_meter() const;
        float get_pixels_per_meter_f() const;

        // Begin a frame: bind FBO, set logical viewport, clear.
        void frame_start();

        // End a frame: unbind FBO, blit to window back-buffer.
        void frame_end();

        // Sprite draw in logical screen space (top-left origin, y-down).
        // screen_pos is the unrotated top-left of the rect.
        // pivot_pixel is the rotation pivot in pixel coords relative to that top-left.
        // rotation_rad is in world-space radians (CCW in y-up).
        void draw_sprite(TextureId texture_id,
                         const Vector2& screen_pos,
                         const Vector2& size,
                         const Vector2& pivot_pixel,
                         float rotation_rad,
                         const Color& tint);

        // Line draw in logical screen space.
        void draw_line(const Vector2& a, const Vector2& b, const Color& color, float thickness);

        // Texture cache.
        TextureId load_texture(const std::filesystem::path& full_path);
        bool unload_texture(TextureId id);
        void get_texture_size(TextureId id, int& out_width, int& out_height) const;

    private:
        bool init_sprite_pipeline();
        bool init_line_pipeline();
        bool init_blit_pipeline();
        bool init_fbo();

        // Low-level GL texture helpers (do not touch the cache).
        static TextureId create_texture_from_pixels(const void* rgba_pixels, int width, int height);
        static void destroy_texture(TextureId id);

        void unload_all_textures();

        void register_cvars(Console& console);
    };
}
