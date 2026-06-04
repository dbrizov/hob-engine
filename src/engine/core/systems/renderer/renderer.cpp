#include "renderer.h"

#include <algorithm>
#include <unordered_set>

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "engine/core/engine_config.h"
#include "engine/core/logging.h"
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
        , m_projection(ortho_top_left(static_cast<float>(m_logical_width),
                                      static_cast<float>(m_logical_height)))
        , m_overlay_projection(ortho_top_left_y_flipped(static_cast<float>(m_logical_width),
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
        if (!init_line_pipeline())
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

        if (m_line_transfer_buffer)
            SDL_ReleaseGPUTransferBuffer(m_gpu_device, m_line_transfer_buffer);
        if (m_line_vbo)
            SDL_ReleaseGPUBuffer(m_gpu_device, m_line_vbo);
        if (m_line_pipeline)
            SDL_ReleaseGPUGraphicsPipeline(m_gpu_device, m_line_pipeline);
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

    uint32_t Renderer::get_logical_width() const {
        return m_logical_width;
    }

    uint32_t Renderer::get_logical_height() const {
        return m_logical_height;
    }

    float Renderer::get_logical_width_f() const {
        return static_cast<float>(m_logical_width);
    }

    float Renderer::get_logical_height_f() const {
        return static_cast<float>(m_logical_height);
    }

    void Renderer::draw_sprite(TextureRef texture,
                               const Vector2& screen_pos,
                               const Vector2& size_pixels,
                               const Vector2& pivot_pixel,
                               float rotation_rad,
                               int z_index,
                               const Material& material) {
        m_pending_sprites.push_back(
            {std::move(texture), screen_pos, size_pixels, pivot_pixel, rotation_rad, z_index, material});
    }

    void Renderer::draw_overlay_sprite(TextureRef texture,
                                       const Vector2& screen_pos,
                                       const Vector2& size_pixels,
                                       const Vector2& pivot_pixel,
                                       float rotation_rad,
                                       const Material& material) {
        m_pending_overlay_sprites.push_back(
            {std::move(texture), screen_pos, size_pixels, pivot_pixel, rotation_rad, 0, material});
    }

    void Renderer::draw_line(const Vector2& start, const Vector2& end, const Color& color, float thickness) {
        // Expand the segment into a screen-aligned quad. Perpendicular extrusion is in
        // logical-pixel space, so the quad has uniform pixel width on the offscreen target.
        const Vector2 delta = end - start;
        const float len = delta.length();
        if (len <= 0.0f) {
            return;
        }

        const float half = std::max(thickness, 1.0f) * 0.5f;
        const Vector2 perp = Vector2(-delta.y, delta.x) / len;
        const Vector2 offset = perp * half;

        const Vector2 p0 = start + offset;
        const Vector2 p1 = start - offset;
        const Vector2 p2 = end + offset;
        const Vector2 p3 = end - offset;

        m_pending_lines.push_back({p0, color});
        m_pending_lines.push_back({p1, color});
        m_pending_lines.push_back({p2, color});
        m_pending_lines.push_back({p2, color});
        m_pending_lines.push_back({p1, color});
        m_pending_lines.push_back({p3, color});
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
}
