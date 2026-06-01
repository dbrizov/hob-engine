#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_gpu.h>

#include "engine/math/color.h"
#include "engine/math/vector2.h"

namespace hob {
    struct EngineConfig;
    class SdlContext;
    class Renderer;
    class Console;

    using TextureId = int32_t;
    constexpr TextureId INVALID_TEXTURE_ID = -1;

    using ShaderId = int32_t;
    constexpr ShaderId INVALID_SHADER_ID = -1;
    constexpr ShaderId DEFAULT_SPRITE_SHADER_ID = 0;

    constexpr SDL_FColor CLEAR_COLOR = SDL_FColor{0.17f, 0.18f, 0.47f, 1.0f};

    // Move-only RAII handle owning one ref count contribution in the Renderer's texture cache.
    // Obtain via Renderer::get_or_load_texture; destructor releases.
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

    struct Material {
        ShaderId shader_id = DEFAULT_SPRITE_SHADER_ID;
        Color tint = Color::white();
    };

    class Renderer {
        struct Sprite {
            TextureId texture_id = INVALID_TEXTURE_ID;
            Vector2 screen_pos;
            Vector2 size_pixels;
            Vector2 pivot_pixel;
            float rotation_rad = 0.0;
            Material material;
        };

        struct LineVertex {
            Vector2 pos;
            Color color;
        };

        struct TextureEntry {
            SDL_GPUTexture* texture;
            uint32_t width;
            uint32_t height;
            std::string path;
            int ref_count;
        };

        const SdlContext& m_sdl_context;
        SDL_GPUDevice* m_gpu_device = nullptr;
        uint32_t m_logical_width;
        uint32_t m_logical_height;
        uint32_t m_pixels_per_meter;

        // SDL_GPU clip-space ortho mapping (0,0)..(w,h) -> (-1,-1)..(+1,+1) with y-down.
        std::array<float, 16> m_projection{};

        // Per-frame batches.
        std::vector<Sprite> m_pending_sprites;
        std::vector<LineVertex> m_pending_lines;

        // Offscreen color target at logical resolution. Sprite pass renders into this;
        // blit pass samples it into the swapchain at window resolution.
        SDL_GPUTexture* m_offscreen_color = nullptr;
        SDL_GPUTextureFormat m_offscreen_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

        // Sprite pipelines indexed by ShaderId, plus path-dedupe map. Slot 0 holds the
        // default pipeline (built from "builtin/shaders/sprite") and is pre-warmed at init,
        // so DEFAULT_SPRITE_SHADER_ID is always valid. Failed builds alias the default id.
        std::vector<SDL_GPUGraphicsPipeline*> m_sprite_pipelines;
        std::unordered_map<std::string, ShaderId> m_shader_path_to_id;
        SDL_GPUBuffer* m_quad_vbo = nullptr;

        // Fullscreen blit pipeline (no VBO; vertex shader synthesizes a triangle from SV_VertexID).
        SDL_GPUGraphicsPipeline* m_blit_pipeline = nullptr;

        // Line pipeline + persistent dynamic VBO and matching upload transfer buffer.
        // Lines drawn in a frame are uploaded once into the VBO via a copy pass on the
        // engine's per-frame command buffer (no fence-wait — both are cycled).
        SDL_GPUGraphicsPipeline* m_line_pipeline = nullptr;
        SDL_GPUBuffer* m_line_vbo = nullptr;
        SDL_GPUTransferBuffer* m_line_transfer_buffer = nullptr;
        // 6 verts per line segment (two triangles): 24576 verts = 4096 lines/frame.
        static constexpr uint32_t MAX_LINE_VERTICES = 24576;

        // Samplers:
        //  sprite: MIN=LINEAR, MAG=NEAREST  (smooth when shrunk, crisp pixel edges when enlarged)
        //  blit:   MIN=LINEAR, MAG=LINEAR   (smooth in both directions when upscaling offscreen → window)
        SDL_GPUSampler* m_sprite_sampler = nullptr;
        SDL_GPUSampler* m_blit_sampler = nullptr;

        // Texture cache.
        std::unordered_map<TextureId, TextureEntry> m_textures;
        std::unordered_map<std::string, TextureId> m_path_to_id;
        TextureId m_next_texture_id = 0;
        bool m_cvar_log_textures = false;

        bool m_shadercross_initialized = false;
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

        /// Draws a textured quad in logical screen space (top-left origin, y-down).
        /// All pixel-valued parameters are in logical pixels — the same space the
        /// orthographic projection is configured in, NOT window pixels.
        void render_sprite(TextureId texture_id,
                           const Vector2& screen_pos,
                           const Vector2& size_pixels,
                           const Vector2& pivot_pixel,
                           float rotation_rad,
                           const Material& material);

        // Resolve a sprite-shader path (relative to assets root, no .vert.hlsl / .frag.hlsl suffix) to a ShaderId.
        // Lazily builds and caches the pipeline on first request.
        // Failed builds alias DEFAULT_SPRITE_SHADER_ID (no retry spam).
        ShaderId get_or_build_sprite_shader(const std::string& path);

        /// Draws a line segment in logical screen space (top-left origin, y-down).
        void render_line(const Vector2& a, const Vector2& b, const Color& color, float thickness);

        // Texture cache.
        // Loads (or returns a cached ref to) a texture by path relative to the assets root.
        TextureRef get_or_load_texture(const std::string& path);

        // Frame recording (called by Engine, which owns the command buffer + swapchain pass).
        // record_world replays queued sprites + lines into the offscreen color target.
        void record_world(SDL_GPUCommandBuffer* cmd);

        // record_blit issues the upscale draw inside the swapchain render pass.
        void record_blit(SDL_GPURenderPass* swap_pass);

    private:
        friend class TextureRef;
        bool unload_texture(TextureId id);

        bool init_offscreen_target();
        bool init_samplers();
        bool init_quad_vbo();
        bool init_default_sprite_pipeline();
        bool init_blit_pipeline();
        bool init_line_pipeline();

        // Builds a sprite pipeline from a shader path (relative to assets root, no suffix).
        // Returns nullptr on failure; caller handles fallback.
        SDL_GPUGraphicsPipeline* build_sprite_pipeline(const std::string& path);

        void register_cvars(Console& console);

        // One-shot transfer-buffer upload of `data` (`size` bytes) into `dst_buffer`.
        // Fences the upload so the buffer is safe to use on the next frame.
        bool upload_buffer(SDL_GPUBuffer* dst_buffer, const void* data, uint32_t size);

        // Same for textures: uploads RGBA8 pixels into `dst_texture` at mip 0, layer 0.
        bool upload_texture_rgba(SDL_GPUTexture* dst_texture, const void* pixels, uint32_t width, uint32_t height);
    };
}
