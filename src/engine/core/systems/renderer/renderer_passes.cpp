#include "renderer.h"

#include <algorithm>
#include <cstring>

#include <SDL3/SDL.h>

namespace hob {
    namespace {
        SDL_FColor to_sdl_color(const Color& c) { return {c.r, c.g, c.b, c.a}; }

        // Must match the HLSL cbuffer layout in sprite.vert.hlsl (cbuffer @ b0, space1).
        // HLSL default packing: each non-vector member can't cross a 16-byte boundary;
        // float2 pairs pack into a single 16-byte slot.
        struct SpriteVSUniforms {
            float proj[16]; // 0..64
            float screen_pos[2]; // 64..72
            float size[2]; // 72..80   (packs with screen_pos)
            float pivot[2]; // 80..88
            float rotation; // 88..92
            float _pad; // 92..96
        };

        static_assert(sizeof(SpriteVSUniforms) == 96);

        // Must match the HLSL cbuffer layout in sprite.frag.hlsl (cbuffer @ b0, space3).
        struct SpriteFSUniforms {
            float tint[4]; // 0..16
            float outline_color[4]; // 16..32
            float outline_width; // 32..36
            float alpha_threshold; // 36..40
            float texel_size[2]; // 40..48
            float time; // 48..52
            float _pad[3]; // 52..64
        };

        static_assert(sizeof(SpriteFSUniforms) == 64);
    }

    void Renderer::render_world_pass() {
        SDL_GPUCommandBuffer* cmd = m_command_buffer;
        // Upload pending line vertices into the persistent line VBO before the render
        // pass starts (copy passes can't run inside a graphics render pass).
        const uint32_t line_vertex_count = static_cast<uint32_t>(
            std::min<size_t>(m_pending_lines.size(), MAX_LINE_VERTICES));

        if (line_vertex_count > 0) {
            const uint32_t bytes = line_vertex_count * sizeof(LineVertex);
            void* map = SDL_MapGPUTransferBuffer(m_gpu_device, m_line_transfer_buffer, true);
            if (map) {
                std::memcpy(map, m_pending_lines.data(), bytes);
                SDL_UnmapGPUTransferBuffer(m_gpu_device, m_line_transfer_buffer);

                SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
                SDL_GPUTransferBufferLocation src{};
                src.transfer_buffer = m_line_transfer_buffer;
                src.offset = 0;
                SDL_GPUBufferRegion dst{};
                dst.buffer = m_line_vbo;
                dst.offset = 0;
                dst.size = bytes;
                SDL_UploadToGPUBuffer(copy, &src, &dst, true);
                SDL_EndGPUCopyPass(copy);
            }
        }

        SDL_GPUColorTargetInfo ct{};
        ct.texture = m_offscreen_color;
        ct.clear_color = to_sdl_color(CLEAR_COLOR);
        ct.load_op = SDL_GPU_LOADOP_CLEAR;
        ct.store_op = SDL_GPU_STOREOP_STORE;

        // Render pass
        {
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &ct, 1, nullptr);
            if (!pass) {
                m_pending_sprites.clear();
                m_pending_lines.clear();
                return;
            }

            // Sprite pipelines
            if (!m_pending_sprites.empty()) {
                std::stable_sort(m_pending_sprites.begin(), m_pending_sprites.end(),
                                 [](const Sprite& a, const Sprite& b) {
                                     if (a.z_index != b.z_index) {
                                         return a.z_index < b.z_index;
                                     }
                                     return a.material.shader_id < b.material.shader_id;
                                 });

                SDL_GPUBufferBinding vb{};
                vb.buffer = m_quad_vbo;
                vb.offset = 0;
                SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

                ShaderId bound_shader = INVALID_SHADER_ID;

                for (const Sprite& sp : m_pending_sprites) {
                    record_sprite(pass, sp, m_projection, bound_shader);
                }
            }

            // Line pipeline
            if (line_vertex_count > 0) {
                SDL_BindGPUGraphicsPipeline(pass, m_line_pipeline);

                SDL_GPUBufferBinding vb{};
                vb.buffer = m_line_vbo;
                vb.offset = 0;
                SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

                SDL_PushGPUVertexUniformData(cmd,
                                             0,
                                             m_projection.data(),
                                             static_cast<uint32_t>(m_projection.size() * sizeof(float)));

                SDL_DrawGPUPrimitives(pass, line_vertex_count, 1, 0, 0);
            }

            SDL_EndGPURenderPass(pass);
        }

        debug_texture_refs();
        debug_sprite_queue();

        m_pending_sprites.clear();
        m_pending_lines.clear();
    }

    void Renderer::render_blit_pass() {
        SDL_GPUColorTargetInfo ct{};
        ct.texture = m_swap_texture;
        ct.load_op = SDL_GPU_LOADOP_DONT_CARE;
        ct.store_op = SDL_GPU_STOREOP_STORE;

        // Render pass
        {
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(m_command_buffer, &ct, 1, nullptr);
            if (!pass) {
                return;
            }

            SDL_BindGPUGraphicsPipeline(pass, m_blit_pipeline);

            SDL_GPUTextureSamplerBinding ts{};
            ts.texture = m_offscreen_color;
            ts.sampler = m_blit_sampler;
            SDL_BindGPUFragmentSamplers(pass, 0, &ts, 1);

            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);

            SDL_EndGPURenderPass(pass);
        }
    }

    void Renderer::render_overlay_pass() {
        if (m_pending_overlay_sprites.empty()) {
            return;
        }

        SDL_GPUColorTargetInfo ct{};
        ct.texture = m_swap_texture;
        ct.load_op = SDL_GPU_LOADOP_LOAD;
        ct.store_op = SDL_GPU_STOREOP_STORE;

        // Render pass
        {
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(m_command_buffer, &ct, 1, nullptr);
            if (!pass) {
                m_pending_overlay_sprites.clear();
                return;
            }

            SDL_GPUBufferBinding vb{};
            vb.buffer = m_quad_vbo;
            vb.offset = 0;
            SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

            ShaderId bound_shader = INVALID_SHADER_ID;

            // Overlay sprites are drawn in push order (no z-sort).
            for (const Sprite& sp : m_pending_overlay_sprites) {
                record_sprite(pass, sp, m_overlay_projection, bound_shader);
            }

            SDL_EndGPURenderPass(pass);
        }

        m_pending_overlay_sprites.clear();
    }

    void Renderer::record_sprite(SDL_GPURenderPass* pass,
                                 const Sprite& sp,
                                 const std::array<float, 16>& projection,
                                 ShaderId& bound_shader) {
        if (!sp.texture || !sp.texture->m_gpu_texture) {
            return;
        }

        if (sp.material.shader_id != bound_shader) {
            SDL_BindGPUGraphicsPipeline(pass, m_sprite_pipelines[sp.material.shader_id]);
            bound_shader = sp.material.shader_id;
        }

        SpriteVSUniforms vsu{};
        std::memcpy(vsu.proj, projection.data(), sizeof(vsu.proj));
        vsu.screen_pos[0] = sp.screen_pos.x;
        vsu.screen_pos[1] = sp.screen_pos.y;
        vsu.size[0] = sp.size_pixels.x;
        vsu.size[1] = sp.size_pixels.y;
        vsu.pivot[0] = sp.pivot_pixel.x;
        vsu.pivot[1] = sp.pivot_pixel.y;
        // World rotation is y-up CCW; logical screen is y-down. Negate so positive
        // world rotation remains visually CCW.
        vsu.rotation = -sp.rotation_rad;
        SDL_PushGPUVertexUniformData(m_command_buffer, 0, &vsu, sizeof(vsu));

        SpriteFSUniforms fsu{};
        fsu.tint[0] = sp.material.tint.r;
        fsu.tint[1] = sp.material.tint.g;
        fsu.tint[2] = sp.material.tint.b;
        fsu.tint[3] = sp.material.tint.a;
        fsu.outline_color[0] = sp.material.outline_color.r;
        fsu.outline_color[1] = sp.material.outline_color.g;
        fsu.outline_color[2] = sp.material.outline_color.b;
        fsu.outline_color[3] = sp.material.outline_color.a;
        fsu.outline_width = sp.material.outline_width;
        fsu.alpha_threshold = sp.material.alpha_threshold;
        const uint32_t tex_w = sp.texture->get_width();
        const uint32_t tex_h = sp.texture->get_height();
        fsu.texel_size[0] = tex_w > 0 ? 1.0f / static_cast<float>(tex_w) : 0.0f;
        fsu.texel_size[1] = tex_h > 0 ? 1.0f / static_cast<float>(tex_h) : 0.0f;
        fsu.time = m_frame_time;
        SDL_PushGPUFragmentUniformData(m_command_buffer, 0, &fsu, sizeof(fsu));

        SDL_GPUTextureSamplerBinding ts{};
        ts.texture = sp.texture->m_gpu_texture;
        ts.sampler = m_sprite_sampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &ts, 1);

        SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
    }
}
