#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_gpu.h>

#include "engine/math/color.h"
#include "engine/math/vector2.h"

#include "material.h"
#include "texture.h"

namespace hob {
    struct EngineConfig;
    class SdlContext;
    class Console;

    constexpr Color CLEAR_COLOR = Color(0.17f, 0.18f, 0.47f, 1.0f);

    class Renderer {
        struct Sprite {
            TextureRef texture;
            Vector2 screen_pos;
            Vector2 size_pixels;
            Vector2 pivot_pixel;
            float rotation_rad = 0.0;
            int z_index = 0;
            Material material;
        };

        struct LineVertex {
            Vector2 pos;
            Color color;
        };

        const SdlContext& m_sdl_context;
        SDL_GPUDevice* m_gpu_device = nullptr;
        uint32_t m_logical_width;
        uint32_t m_logical_height;

        bool m_shadercross_initialized = false;
        bool m_is_initialized = false;

        // Seconds-since-play-start, refreshed by Engine each frame
        // and pushed to fragment cbuffers so shaders can animate.
        float m_frame_time = 0.0f;

        // SDL_GPU clip-space ortho mapping (0,0)..(w,h) -> (-1,-1)..(+1,+1) with y-down.
        std::array<float, 16> m_projection{};
        // Y-flipped variant used by render_overlay_pass (swap target has opposite NDC y).
        std::array<float, 16> m_overlay_projection{};

        // Per-frame batches.
        std::vector<Sprite> m_pending_sprites;
        std::vector<Sprite> m_pending_overlay_sprites;
        std::vector<LineVertex> m_pending_lines;

        // Per-frame GPU state, valid between acquire_command_buffer() and submit/cancel.
        SDL_GPUCommandBuffer* m_command_buffer = nullptr;
        SDL_GPUTexture* m_swap_texture = nullptr;

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

        // Texture cache. Holds weak refs keyed by normalized asset path, so unused
        // textures are released as soon as their last shared_ptr<Texture> is dropped.
        std::unordered_map<std::string, std::weak_ptr<Texture>> m_textures;

        // CVars.
        bool m_cvar_log_texture_ref = false;
        bool m_cvar_show_texture_refs = false;

        bool m_cvar_log_sprite_queue = false;
        bool m_cvar_show_sprite_queue = false;

    public:
        Renderer(const EngineConfig& config, const SdlContext& sdl_context, Console& console);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool is_initialized() const;

        void set_frame_time(float time);

        uint32_t get_logical_width() const;
        uint32_t get_logical_height() const;
        float get_logical_width_f() const;
        float get_logical_height_f() const;

        /// Draws a textured quad in logical screen space (top-left origin, y-down).
        /// All pixel-valued parameters are in logical pixels — the same space the
        /// orthographic projection is configured in, NOT window pixels.
        void draw_sprite(TextureRef texture,
                         const Vector2& screen_pos,
                         const Vector2& size_pixels,
                         const Vector2& pivot_pixel,
                         float rotation_rad,
                         int z_index,
                         const Material& material);

        /// Queues a sprite to be drawn in the overlay pass — on top of the world AND ImGui.
        /// Same coordinate space as draw_sprite (logical screen pixels). No z_index: overlays are drawn in push order.
        void draw_overlay_sprite(TextureRef texture,
                                 const Vector2& screen_pos,
                                 const Vector2& size_pixels,
                                 const Vector2& pivot_pixel,
                                 float rotation_rad,
                                 const Material& material);

        /// Draws a line segment in logical screen space (top-left origin, y-down).
        void draw_line(const Vector2& start, const Vector2& end, const Color& color, float thickness);

        // Texture cache.
        // Loads (or returns a cached ref to) a texture by path relative to the assets root.
        TextureRef get_or_load_texture(const std::string& path);

        // Resolve a sprite-shader path (relative to assets root, no .vert.hlsl / .frag.hlsl suffix) to a ShaderId.
        // Lazily builds and caches the pipeline on first request.
        // Failed builds alias DEFAULT_SPRITE_SHADER_ID (no retry spam).
        ShaderId get_or_build_sprite_shader(const std::string& path);

        // Per-frame GPU lifecycle. Engine calls acquire_command_buffer() at frame start;
        // if it returns true the swapchain is usable and the engine records work + submits,
        // otherwise it cancels.
        bool acquire_command_buffer();
        void submit_command_buffer();
        void cancel_command_buffer();

        SDL_GPUCommandBuffer* get_command_buffer() const;
        SDL_GPUTexture* get_swap_texture() const;

        // Renders queued sprites + lines into the offscreen color target.
        void render_world_pass();

        // Opens a render pass on the swapchain texture, upscales the offscreen target and closes the pass.
        void render_blit_pass();

        // Opens a render pass on the swapchain texture with LOAD_OP_LOAD and draws the
        // queued overlay sprites on top of whatever's already there (world + ImGui).
        void render_overlay_pass();

    private:
        friend class Texture;
        // Called by Texture::~Texture to release the GPU handle and drop the cache entry.
        void release_texture(const Texture& texture);

        bool init_offscreen_target();
        bool init_samplers();
        bool init_quad_vbo();
        bool init_default_sprite_pipeline();
        bool init_blit_pipeline();
        bool init_line_pipeline();

        // Builds a sprite pipeline from a shader path (relative to assets root, no suffix).
        // Returns nullptr on failure; caller handles fallback.
        SDL_GPUGraphicsPipeline* build_sprite_pipeline(const std::string& path);

        // Records one sprite's draw commands into `pass`. Reads m_command_buffer for
        // uniform pushes. Updates `bound_shader` so callers can skip redundant pipeline
        // binds across a batch of sprites.
        void record_sprite(SDL_GPURenderPass* pass,
                           const Sprite& sp,
                           const std::array<float, 16>& projection,
                           ShaderId& bound_shader);

        // One-shot transfer-buffer upload of `data` (`size` bytes) into `dst_buffer`.
        // Fences the upload so the buffer is safe to use on the next frame.
        bool upload_buffer(SDL_GPUBuffer* dst_buffer, const void* data, uint32_t size);

        // Same for textures: uploads RGBA8 pixels into `dst_texture` at mip 0, layer 0.
        bool upload_texture_rgba(SDL_GPUTexture* dst_texture, const void* pixels, uint32_t width, uint32_t height);

        void debug_texture_refs();
        void debug_sprite_queue();

        void register_cvars(Console& console);
    };
}
