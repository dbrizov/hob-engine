#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_gpu.h>

#include "engine/math/color.h"
#include "engine/math/vector2.h"

#include "font.h"
#include "material.h"
#include "texture.h"

namespace hob {
    struct EngineConfig;
    class SdlContext;
    class Console;

    class Renderer {
        static constexpr Color CLEAR_COLOR = Color(0.17f, 0.18f, 0.47f, 1.0f);

        // 6 verts per line segment (two triangles): 65536 verts = ~10,922 lines/frame.
        static constexpr uint32_t MAX_DEBUG_LINE_VERTICES = 65536;

        // 4 verts and 6 indices per glyph quad.
        static constexpr uint32_t MAX_DEBUG_TEXT_GLYPHS = 4096;
        static constexpr uint32_t MAX_DEBUG_TEXT_VERTICES = MAX_DEBUG_TEXT_GLYPHS * 4;
        static constexpr uint32_t MAX_DEBUG_TEXT_INDICES = MAX_DEBUG_TEXT_GLYPHS * 6;
        // Drop-shadow drawn behind every debug text glyph, in window pixels. Offset 0 disables it.
        static constexpr Color DEBUG_TEXT_SHADOW_COLOR = Color::black();
        static constexpr Vector2 DEBUG_TEXT_SHADOW_OFFSET = Vector2(1.0f, 1.0f);

        static constexpr float DEBUG_FONT_SIZE_PX = 13.0f;

        // Builtin asset paths, relative to the assets root.
        static constexpr std::string_view BUILTIN_SHADERS_DIR = "builtin/shaders";
        static constexpr std::string_view DEFAULT_SPRITE_SHADER = "builtin/shaders/sprite";
        static constexpr std::string_view DEBUG_FONT_PATH = "builtin/fonts/jetbrains_mono_bold.ttf";

        struct Sprite {
            TextureRef texture;
            Vector2 screen_pos;
            Vector2 size_pixels;
            Vector2 pivot_pixels;
            float rotation_rad = 0.0;
            int z_index = 0;
            Material material;
        };

        struct DebugLineVertex {
            Vector2 screen_pos;
            Color color;
        };

        struct DebugTextVertex {
            Vector2 screen_pos;
            Vector2 uv;
            Color color;
        };

        // --- Lifecycle ---

        const SdlContext& m_sdl_context;
        SDL_GPUDevice* m_gpu_device = nullptr;
        uint32_t m_logical_width;
        uint32_t m_logical_height;

        bool m_shadercross_initialized = false;
        bool m_is_initialized = false;

        // Seconds-since-play-start, refreshed by Engine each frame
        // and pushed to fragment cbuffers so shaders can animate.
        float m_frame_time = 0.0f;

        // Projection used when rendering into the offscreen color target. SDL_GPU clip-space
        // ortho mapping (0,0)..(w,h) -> (-1,-1)..(+1,+1) with y-down. Input is logical pixels.
        std::array<float, 16> m_offscreen_projection{};
        // Projection used when rendering directly into the swapchain. Same logical-pixel input
        // range as m_offscreen_projection, but y-flipped because the swap target has the opposite NDC y convention.
        std::array<float, 16> m_swapchain_projection{};

        // --- Per-frame state ---

        // Valid between acquire_command_buffer() and submit/cancel.
        SDL_GPUCommandBuffer* m_command_buffer = nullptr;
        SDL_GPUTexture* m_swap_texture = nullptr;

        // Per-frame batches, drained by the render passes.
        std::vector<Sprite> m_pending_sprites;
        std::vector<Sprite> m_pending_overlay_sprites;
        std::vector<DebugLineVertex> m_pending_debug_line_vertices;
        std::vector<DebugTextVertex> m_pending_debug_text_vertices;
        std::vector<uint16_t> m_pending_debug_text_indices;

        // --- Resources (texture cache) ---

        // Holds weak refs keyed by normalized asset path, so unused textures are
        // released as soon as their last shared_ptr<Texture> is dropped.
        std::unordered_map<std::string, std::weak_ptr<Texture>> m_textures;

        // --- Pipelines ---

        // Offscreen color target at logical resolution. World pass renders into this.
        // Blit pass samples it into the swapchain at window resolution.
        SDL_GPUTexture* m_offscreen_color = nullptr;
        SDL_GPUTextureFormat m_offscreen_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

        // Sprite pipelines indexed by ShaderId, plus path-dedupe map. Slot 0 holds the
        // default pipeline (built from DEFAULT_SPRITE_SHADER) and is pre-warmed at init,
        // so DEFAULT_SPRITE_SHADER_ID is always valid. Failed builds alias the default id.
        std::vector<SDL_GPUGraphicsPipeline*> m_sprite_pipelines;
        std::unordered_map<std::string, ShaderId> m_shader_path_to_id;
        SDL_GPUBuffer* m_quad_vbo = nullptr;

        // Fullscreen blit pipeline (no VBO; vertex shader synthesizes a triangle from SV_VertexID).
        SDL_GPUGraphicsPipeline* m_blit_pipeline = nullptr;

        // Debug-line pipeline + persistent dynamic VBO and matching upload transfer buffer.
        // Lines drawn in a frame are uploaded once into the VBO via a copy pass on the
        // engine's per-frame command buffer (no fence-wait — both are cycled).
        SDL_GPUGraphicsPipeline* m_debug_line_pipeline = nullptr;
        SDL_GPUBuffer* m_debug_line_vbo = nullptr;
        SDL_GPUTransferBuffer* m_debug_line_transfer_buffer = nullptr;

        // Debug-text pipeline + persistent dynamic VBO/IBO and matching upload transfer buffer.
        SDL_GPUGraphicsPipeline* m_debug_text_pipeline = nullptr;
        SDL_GPUBuffer* m_debug_text_vbo = nullptr;
        SDL_GPUBuffer* m_debug_text_ibo = nullptr;
        SDL_GPUTransferBuffer* m_debug_text_vbo_transfer = nullptr;
        SDL_GPUTransferBuffer* m_debug_text_ibo_transfer = nullptr;
        SDL_GPUSampler* m_debug_text_sampler = nullptr;
        Font m_debug_font;
        float m_debug_font_baked_inverse_pixel_density = 1.0f;

        // Samplers:
        //  sprite: MIN=LINEAR, MAG=NEAREST  (smooth when shrunk, crisp pixel edges when enlarged)
        //  blit:   MIN=LINEAR, MAG=LINEAR   (smooth in both directions when upscaling offscreen → window)
        SDL_GPUSampler* m_sprite_sampler = nullptr;
        SDL_GPUSampler* m_blit_sampler = nullptr;

        // --- Debug cvars ---

        bool m_cvar_log_texture_ref = false;
        bool m_cvar_show_texture_refs = false;

        bool m_cvar_log_sprite_queue = false;
        bool m_cvar_show_sprite_queue = false;

    public:
        // --- Lifecycle (renderer.cpp) ---

        Renderer(const EngineConfig& config, const SdlContext& sdl_context, Console& console);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool is_initialized() const;

        void set_frame_time(float time);

        Vector2 get_logical_size() const;

        // --- Per-frame command buffer (renderer.cpp) ---

        // Engine calls acquire_command_buffer() at frame start; if it returns true the
        // swapchain is usable and the engine records work + submits, otherwise it cancels.
        bool acquire_command_buffer();
        void submit_command_buffer();
        void cancel_command_buffer();

        SDL_GPUCommandBuffer* get_command_buffer() const;
        SDL_GPUTexture* get_swap_texture() const;

        // --- Draw queueing (renderer.cpp) ---

        /// Draws a textured quad in logical screen space (top-left origin, y-down).
        /// All pixel-valued parameters are in logical pixels — the same space the
        /// orthographic projection is configured in, NOT window pixels.
        void draw_sprite(TextureRef texture,
                         const Vector2& screen_pos,
                         const Vector2& size_pixels,
                         const Vector2& pivot_pixels,
                         float rotation_rad,
                         int z_index,
                         const Material& material);

        /// Queues a sprite to be drawn in the overlay pass — on top of the world AND ImGui.
        /// Same coordinate space as draw_sprite (logical screen pixels). No z_index: overlays are drawn in push order.
        void draw_overlay_sprite(TextureRef texture,
                                 const Vector2& screen_pos,
                                 const Vector2& size_pixels,
                                 const Vector2& pivot_pixels,
                                 float rotation_rad,
                                 const Material& material);

        /// Draws a debug line segment in logical screen space (top-left origin, y-down).
        void draw_debug_line(const Vector2& screen_start,
                             const Vector2& screen_end,
                             const Color& color,
                             float thickness_pixels);

        /// Draws a debug text string in logical screen space (top-left origin, y-down).
        void draw_debug_text(const Vector2& screen_pos, std::string_view text, const Color& color, float scale);

        /// Returns the line height of the debug font in logical pixels.
        int get_debug_font_line_height() const;

        // --- Render passes (renderer_passes.cpp) ---

        // Renders queued sprites into the offscreen color target.
        void render_world_pass();

        // Opens a render pass on the swapchain target, upscales the offscreen target and closes the pass.
        void render_blit_pass();

        // Renders queued debug lines into the swapchain target.
        void render_debug_lines_pass();

        // Renders queued debug text glyphs into the swapchain target.
        void render_debug_text_pass();

        // Renders queued overlay sprites into the swapchain target.
        void render_overlay_pass();

        // --- Resources (renderer_resources.cpp) ---

        // Loads (or returns a cached ref to) a texture by path relative to the assets root.
        TextureRef get_or_load_texture(const std::string& path);

        // --- Pipelines (renderer_pipelines.cpp) ---

        // Resolve a sprite-shader path (relative to assets root, no .vert.hlsl / .frag.hlsl suffix) to a ShaderId.
        // Lazily builds and caches the pipeline on first request.
        // Failed builds alias DEFAULT_SPRITE_SHADER_ID (no retry spam).
        ShaderId get_or_build_sprite_shader(const std::string& path);

    private:
        friend class Texture;
        friend class Font;

        // --- Resources (renderer_resources.cpp) ---

        // Called by Texture::~Texture to release the GPU handle and drop the cache entry.
        void release_texture(const Texture& texture);

        // One-shot transfer-buffer upload of `data` (`size` bytes) into `dst_buffer`.
        // Fences the upload so the buffer is safe to use on the next frame.
        bool upload_buffer(SDL_GPUBuffer* dst_buffer, const void* data, uint32_t size);

        // Same for textures: uploads RGBA8 pixels into `dst_texture` at mip 0, layer 0.
        bool upload_texture_rgba(SDL_GPUTexture* dst_texture, const void* pixels, uint32_t width, uint32_t height);

        // --- Pipelines (renderer_pipelines.cpp) ---

        bool init_offscreen_target();
        bool init_samplers();
        bool init_quad_vbo();
        bool init_default_sprite_pipeline();
        bool init_blit_pipeline();
        bool init_debug_line_pipeline();
        bool init_debug_text_pipeline();
        bool init_debug_font();

        // Builds a sprite pipeline from a shader path (relative to assets root, no suffix).
        // Returns nullptr on failure; caller handles fallback.
        SDL_GPUGraphicsPipeline* build_sprite_pipeline(const std::string& path);

        // --- Render passes (renderer_passes.cpp) ---

        // Records one sprite's draw commands into `pass`. Reads m_command_buffer for
        // uniform pushes. Updates `bound_shader` so callers can skip redundant pipeline
        // binds across a batch of sprites.
        void record_sprite(SDL_GPURenderPass* pass,
                           const Sprite& sp,
                           const std::array<float, 16>& projection,
                           ShaderId& bound_shader);

        // --- Debug (renderer_debug.cpp) ---

        void register_cvars(Console& console);
        void debug_texture_refs();
        void debug_sprite_queue();
    };
}
