#include "renderer.h"

#include <algorithm>
#include <unordered_set>

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "engine/core/engine_config.h"
#include "engine/core/debug.h"
#include "engine/core/systems/console.h"
#include "engine/core/systems/sdl_context.h"

namespace hob {
    namespace {
        // SDL_GPU clip space is y-down (Vulkan/D3D convention). Logical screen space is also
        // y-down (top-left origin), so we map x:[0,w]->[-1,+1] and y:[0,h]->[-1,+1]
        std::array<float, 16> ortho_top_left(float w, float h) {
            std::array<float, 16> m{};
            m[0] = 2.0f / w;
            m[5] = 2.0f / h;
            m[10] = 1.0f;
            m[12] = -1.0f;
            m[13] = -1.0f;
            m[15] = 1.0f;
            return m;
        }

        // Same logical-pixel input range as ortho_top_left, but maps y:[0,h] -> [+1,-1].
        // Used for passes that target the swapchain directly (overlay) — the swap's NDC
        // y convention is opposite to the offscreen target's, and without flipping here
        // the overlay would render vertically mirrored relative to the world.
        std::array<float, 16> ortho_top_left_y_flipped(float w, float h) {
            std::array<float, 16> m{};
            m[0] = 2.0f / w;
            m[5] = -2.0f / h;
            m[10] = 1.0f;
            m[12] = -1.0f;
            m[13] = 1.0f;
            m[15] = 1.0f;
            return m;
        }
    }

    Renderer::Renderer(const EngineConfig& config, const SdlContext& sdl_context, Console& console)
        : m_sdl_context(sdl_context)
        , m_gpu_device(sdl_context.get_gpu_device())
        , m_logical_width(config.graphics_config.logical_resolution_width)
        , m_logical_height(config.graphics_config.logical_resolution_height)
        , m_offscreen_projection(ortho_top_left(static_cast<float>(m_logical_width),
                                                static_cast<float>(m_logical_height)))
        , m_swapchain_projection(ortho_top_left_y_flipped(static_cast<float>(m_logical_width),
                                                          static_cast<float>(m_logical_height))) {
        if (!m_gpu_device) {
            debug::log_error("Renderer init failed: GPU device is null");
            return;
        }

        if (!SDL_ShaderCross_Init()) {
            debug::log_error("SDL_ShaderCross_Init failed: {}", SDL_GetError());
            return;
        }
        m_shadercross_initialized = true;

        if (!init_offscreen_target())
            return;
        if (!init_samplers())
            return;
        if (!init_quad_vbo())
            return;
        if (!init_default_sprite_pipeline())
            return;
        if (!init_blit_pipeline())
            return;
        if (!init_debug_line_pipeline())
            return;
        if (!init_debug_text_pipeline())
            return;
        if (!init_debug_font())
            return;

        register_cvars(console);

        m_is_initialized = true;
        debug::log("Renderer initialized (logical {}x{})", m_logical_width, m_logical_height);
    }

    Renderer::~Renderer() {
        // Defensive sweep: with the engine's subsystem destruction order, every TextureRef
        // holder dies before the Renderer, so the weak refs here should already be expired.
        // If anything is still alive, detach it from the renderer and release its GPU handle
        // directly so Texture::~Texture (which would call back into us) becomes a no-op.
        for (auto& [path, weak] : m_textures) {
            if (auto tex = weak.lock()) {
                debug::log_error(
                    "Renderer::~Renderer: texture '{}' still has {} holder(s) at shutdown — destruction order is wrong",
                    path,
                    tex.use_count() - 1);

                if (tex->m_gpu_texture) {
                    SDL_ReleaseGPUTexture(m_gpu_device, tex->m_gpu_texture);
                    tex->m_gpu_texture = nullptr;
                }

                tex->m_renderer = nullptr;
            }
        }
        m_textures.clear();

        // Debug font owns its atlas texture; release before the GPU device goes away.
        m_debug_font.shutdown();

        if (m_debug_text_sampler)
            SDL_ReleaseGPUSampler(m_gpu_device, m_debug_text_sampler);
        if (m_debug_text_ibo_transfer)
            SDL_ReleaseGPUTransferBuffer(m_gpu_device, m_debug_text_ibo_transfer);
        if (m_debug_text_vbo_transfer)
            SDL_ReleaseGPUTransferBuffer(m_gpu_device, m_debug_text_vbo_transfer);
        if (m_debug_text_ibo)
            SDL_ReleaseGPUBuffer(m_gpu_device, m_debug_text_ibo);
        if (m_debug_text_vbo)
            SDL_ReleaseGPUBuffer(m_gpu_device, m_debug_text_vbo);
        if (m_debug_text_pipeline)
            SDL_ReleaseGPUGraphicsPipeline(m_gpu_device, m_debug_text_pipeline);

        if (m_debug_line_transfer_buffer)
            SDL_ReleaseGPUTransferBuffer(m_gpu_device, m_debug_line_transfer_buffer);
        if (m_debug_line_vbo)
            SDL_ReleaseGPUBuffer(m_gpu_device, m_debug_line_vbo);
        if (m_debug_line_pipeline)
            SDL_ReleaseGPUGraphicsPipeline(m_gpu_device, m_debug_line_pipeline);
        if (m_blit_pipeline)
            SDL_ReleaseGPUGraphicsPipeline(m_gpu_device, m_blit_pipeline);

        // Sprite pipelines: failed builds alias the default-slot pointer, so dedupe by
        // pointer identity before releasing to avoid double-free.
        std::unordered_set<SDL_GPUGraphicsPipeline*> released_pipelines;
        for (SDL_GPUGraphicsPipeline* pipeline : m_sprite_pipelines) {
            if (pipeline && released_pipelines.insert(pipeline).second) {
                SDL_ReleaseGPUGraphicsPipeline(m_gpu_device, pipeline);
            }
        }
        m_sprite_pipelines.clear();
        m_shader_path_to_id.clear();

        if (m_quad_vbo)
            SDL_ReleaseGPUBuffer(m_gpu_device, m_quad_vbo);
        if (m_blit_sampler)
            SDL_ReleaseGPUSampler(m_gpu_device, m_blit_sampler);
        if (m_sprite_sampler)
            SDL_ReleaseGPUSampler(m_gpu_device, m_sprite_sampler);
        if (m_offscreen_color)
            SDL_ReleaseGPUTexture(m_gpu_device, m_offscreen_color);

        if (m_shadercross_initialized) {
            SDL_ShaderCross_Quit();
        }

        debug::log("Renderer uninitialized");
    }

    bool Renderer::is_initialized() const {
        return m_is_initialized;
    }

    void Renderer::set_frame_time(float time) {
        m_frame_time = time;
    }

    Vector2 Renderer::get_logical_size() const {
        return Vector2(static_cast<float>(m_logical_width), static_cast<float>(m_logical_height));
    }

    bool Renderer::acquire_command_buffer() {
        m_command_buffer = SDL_AcquireGPUCommandBuffer(m_gpu_device);
        m_swap_texture = nullptr;

        const bool ok = SDL_WaitAndAcquireGPUSwapchainTexture(
                            m_command_buffer, m_sdl_context.get_window(),
                            &m_swap_texture, nullptr, nullptr)
                        && m_swap_texture != nullptr;
        return ok;
    }

    void Renderer::submit_command_buffer() {
        SDL_SubmitGPUCommandBuffer(m_command_buffer);
        m_command_buffer = nullptr;
        m_swap_texture = nullptr;
    }

    void Renderer::cancel_command_buffer() {
        SDL_CancelGPUCommandBuffer(m_command_buffer);
        m_command_buffer = nullptr;
        m_swap_texture = nullptr;
    }

    SDL_GPUCommandBuffer* Renderer::get_command_buffer() const {
        return m_command_buffer;
    }

    SDL_GPUTexture* Renderer::get_swap_texture() const {
        return m_swap_texture;
    }

    void Renderer::draw_sprite(TextureRef texture,
                               const Vector2& screen_pos,
                               const Vector2& size_pixels,
                               const Vector2& pivot_pixels,
                               float rotation_rad,
                               int z_index,
                               const Material& material) {
        m_pending_sprites.push_back(
            {std::move(texture), screen_pos, size_pixels, pivot_pixels, rotation_rad, z_index, material});
    }

    void Renderer::draw_overlay_sprite(TextureRef texture,
                                       const Vector2& screen_pos,
                                       const Vector2& size_pixels,
                                       const Vector2& pivot_pixels,
                                       float rotation_rad,
                                       const Material& material) {
        m_pending_overlay_sprites.push_back(
            {std::move(texture), screen_pos, size_pixels, pivot_pixels, rotation_rad, 0, material});
    }

    void Renderer::draw_debug_line(const Vector2& screen_start,
                                   const Vector2& screen_end,
                                   const Color& color,
                                   float thickness_pixels) {
        const Vector2 delta = screen_end - screen_start;
        const float len = delta.length();
        if (len <= 0.0f) {
            return;
        }

        const float half = std::max(thickness_pixels, 1.0f) * 0.5f;
        const Vector2 perp = Vector2(-delta.y, delta.x) / len;
        const Vector2 offset = perp * half;

        const Vector2 p0 = screen_start + offset;
        const Vector2 p1 = screen_start - offset;
        const Vector2 p2 = screen_end + offset;
        const Vector2 p3 = screen_end - offset;

        m_pending_debug_line_vertices.push_back({p0, color});
        m_pending_debug_line_vertices.push_back({p1, color});
        m_pending_debug_line_vertices.push_back({p2, color});
        m_pending_debug_line_vertices.push_back({p2, color});
        m_pending_debug_line_vertices.push_back({p1, color});
        m_pending_debug_line_vertices.push_back({p3, color});
    }

    void Renderer::draw_debug_text(const Vector2& screen_pos, std::string_view text, const Color& color, float scale) {
        if (!m_debug_font.is_initialized() || text.empty()) {
            return;
        }

        float pen_x = screen_pos.x;
        const float pen_y = screen_pos.y;

        for (char c : text) {
            const uint32_t cp = static_cast<uint32_t>(static_cast<unsigned char>(c));
            const Glyph* g = m_debug_font.get_glyph(cp);
            if (!g) {
                continue;
            }

            // Don't tessellate quads for whitespace glyphs (no ink). Still advances the pen.
            if (g->width > 0 && g->height > 0) {
                if (m_pending_debug_text_vertices.size() + 4 > MAX_DEBUG_TEXT_VERTICES) {
                    break;
                }

                const float x0 = pen_x + static_cast<float>(g->offset_x) * scale;
                const float y0 = pen_y + static_cast<float>(g->offset_y) * scale;
                const float x1 = x0 + static_cast<float>(g->width) * scale;
                const float y1 = y0 + static_cast<float>(g->height) * scale;

                const uint16_t base = static_cast<uint16_t>(m_pending_debug_text_vertices.size());

                m_pending_debug_text_vertices.push_back({Vector2(x0, y0), Vector2(g->u0, g->v0), color});
                m_pending_debug_text_vertices.push_back({Vector2(x1, y0), Vector2(g->u1, g->v0), color});
                m_pending_debug_text_vertices.push_back({Vector2(x0, y1), Vector2(g->u0, g->v1), color});
                m_pending_debug_text_vertices.push_back({Vector2(x1, y1), Vector2(g->u1, g->v1), color});

                m_pending_debug_text_indices.push_back(base + 0);
                m_pending_debug_text_indices.push_back(base + 2);
                m_pending_debug_text_indices.push_back(base + 1);
                m_pending_debug_text_indices.push_back(base + 1);
                m_pending_debug_text_indices.push_back(base + 2);
                m_pending_debug_text_indices.push_back(base + 3);
            }

            pen_x += static_cast<float>(g->advance) * scale;
        }
    }

    int Renderer::get_debug_font_line_height() const {
        return m_debug_font.get_line_height();
    }
}
