#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "engine/math/color.h"
#include "engine/math/constants.h"
#include "engine/math/vector2.h"

namespace hob {
    struct EngineConfig;
    class SdlContext;
    class Renderer;
    class Console;

    constexpr const char* GLSL_VERSION = "#version 330 core\n";

    using TextureId = uint32_t;
    constexpr TextureId INVALID_TEXTURE_ID = MAX_UINT32;

    // Move-only RAII handle owning one ref count contribution in the Renderer's texture cache.
    // Obtain via Renderer::load_texture; destructor releases.
    class TextureRef {
        Renderer* m_renderer = nullptr;
        TextureId m_id = INVALID_TEXTURE_ID;
        uint32_t m_width = 0;
        uint32_t m_height = 0;

        friend class Renderer;
        TextureRef(Renderer& renderer, TextureId id, uint32_t width, uint32_t height);

    public:
        TextureRef() = default;
        ~TextureRef();

        TextureRef(const TextureRef&) = delete;
        TextureRef& operator=(const TextureRef&) = delete;

        TextureRef(TextureRef&& other) noexcept;
        TextureRef& operator=(TextureRef&& other) noexcept;

        void reset();

        bool is_valid() const;
        TextureId get_id() const;
        uint32_t get_width() const;
        uint32_t get_height() const;
    };

    class Renderer {
        struct TextureEntry {
            uint32_t width;
            uint32_t height;
            std::string path;
            int ref_count;

            TextureEntry(uint32_t width_, uint32_t height_, std::string path_, int ref_count_)
                : width(width_)
                , height(height_)
                , path(std::move(path_))
                , ref_count(ref_count_) {
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
        Renderer(const EngineConfig& config, const SdlContext& sdl_context, Console& console);
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

        /// Draws a textured quad in logical screen space (top-left origin, y-down).
        /// All pixel-valued parameters are in logical pixels — the same space the
        /// orthographic projection is configured in, NOT window pixels.
        /// @param texture_id   Texture to sample. Must be a valid id from load_texture().
        /// @param screen_pos   Unrotated top-left corner of the destination rect, in logical pixels.
        /// @param size_pixels  Destination rect size in logical pixels (width, height).
        /// @param pivot_pixel  Rotation pivot in logical pixels, relative to screen_pos (top-left).
        /// @param rotation_rad World-space rotation in radians (CCW in y-up world). Internally
        ///                     negated to keep visual CCW under the y-down screen projection.
        /// @param tint         RGBA multiplied with the sampled texel.
        void draw_sprite(TextureId texture_id,
                         const Vector2& screen_pos,
                         const Vector2& size_pixels,
                         const Vector2& pivot_pixel,
                         float rotation_rad,
                         const Color& tint);

        /// Draws a line segment in logical screen space (top-left origin, y-down).
        /// @param a         Start point in logical pixels.
        /// @param b         End point in logical pixels.
        /// @param color     RGBA line color.
        /// @param thickness Line width in pixels. Values below 1.0 are clamped to 1.0
        ///                  (the GL minimum); driver support above ~1.0 is not guaranteed.
        void draw_line(const Vector2& a, const Vector2& b, const Color& color, float thickness);

        // Texture cache.
        TextureRef load_texture(const std::filesystem::path& full_path);

    private:
        friend class TextureRef;
        bool unload_texture(TextureId id);

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
