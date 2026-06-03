#include "renderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <unordered_set>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <imgui.h>

#include "engine/core/engine_config.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
#include "engine/core/systems/console.h"
#include "engine/core/systems/sdl_context.h"

namespace hob {
    namespace {
        SDL_FColor to_sdl_color(const Color& c) { return {c.r, c.g, c.b, c.a}; }

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
        };

        static_assert(sizeof(SpriteFSUniforms) == 48);

        std::string read_text_file(const std::filesystem::path& path) {
            std::ifstream f(path, std::ios::binary | std::ios::ate);
            if (!f) {
                return {};
            }

            const std::streamsize size = f.tellg();
            f.seekg(0);
            std::string contents(static_cast<size_t>(size), '\0');
            f.read(contents.data(), size);
            return contents;
        }

        SDL_GPUShader* load_shader(SDL_GPUDevice* device,
                                   const std::filesystem::path& hlsl_path,
                                   SDL_ShaderCross_ShaderStage stage) {
            const std::string source = read_text_file(hlsl_path);
            if (source.empty()) {
                debug::log_error("Failed to read shader: {}", hlsl_path.string());
                return nullptr;
            }

            SDL_ShaderCross_HLSL_Info hlsl_info{};
            hlsl_info.source = source.c_str();
            hlsl_info.entrypoint = "main";
            hlsl_info.shader_stage = stage;

            size_t spirv_size = 0;
            void* spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_size);
            if (!spirv) {
                debug::log_error("CompileSPIRVFromHLSL failed for {}: {}", hlsl_path.string(), SDL_GetError());
                return nullptr;
            }

            SDL_ShaderCross_GraphicsShaderMetadata* meta =
                SDL_ShaderCross_ReflectGraphicsSPIRV(static_cast<Uint8*>(spirv), spirv_size, 0);
            if (!meta) {
                debug::log_error("ReflectGraphicsSPIRV failed for {}: {}", hlsl_path.string(), SDL_GetError());
                SDL_free(spirv);
                return nullptr;
            }

            SDL_ShaderCross_SPIRV_Info sp_info{};
            sp_info.bytecode = static_cast<Uint8*>(spirv);
            sp_info.bytecode_size = spirv_size;
            sp_info.entrypoint = "main";
            sp_info.shader_stage = stage;

            SDL_GPUShader* shader =
                SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(device, &sp_info, &meta->resource_info, 0);

            SDL_free(spirv);
            SDL_free(meta);

            if (!shader) {
                debug::log_error("CompileGraphicsShaderFromSPIRV failed for {}: {}",
                                 hlsl_path.string(),
                                 SDL_GetError());
                return nullptr;
            }

            return shader;
        }
    }

    // Texture
    Texture::Texture(Renderer& renderer,
                     SDL_GPUTexture* gpu_texture,
                     uint32_t width,
                     uint32_t height,
                     std::string path)
        : m_renderer(&renderer)
        , m_gpu_texture(gpu_texture)
        , m_width(width)
        , m_height(height)
        , m_path(std::move(path)) {
    }

    Texture::~Texture() {
        if (m_renderer != nullptr) {
            m_renderer->release_texture(*this);
        }
    }

    uint32_t Texture::get_width() const {
        return m_width;
    }

    uint32_t Texture::get_height() const {
        return m_height;
    }

    const std::string& Texture::get_path() const {
        return m_path;
    }

    // Renderer
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

    void Renderer::draw_line(const Vector2& a, const Vector2& b, const Color& color, float thickness) {
        // Expand the segment into a screen-aligned quad. Perpendicular extrusion is in
        // logical-pixel space, so the quad has uniform pixel width on the offscreen target.
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.0f) {
            return;
        }

        const float half = std::max(thickness, 1.0f) * 0.5f;
        const float nx = -dy / len * half;
        const float ny = dx / len * half;

        const Vector2 p0{a.x + nx, a.y + ny};
        const Vector2 p1{a.x - nx, a.y - ny};
        const Vector2 p2{b.x + nx, b.y + ny};
        const Vector2 p3{b.x - nx, b.y - ny};

        m_pending_lines.push_back({p0, color});
        m_pending_lines.push_back({p1, color});
        m_pending_lines.push_back({p2, color});
        m_pending_lines.push_back({p2, color});
        m_pending_lines.push_back({p1, color});
        m_pending_lines.push_back({p3, color});
    }

    TextureRef Renderer::get_or_load_texture(const std::string& path) {
        const std::string key = std::filesystem::path(path).lexically_normal().string();

        auto tex_it = m_textures.find(key);
        if (tex_it != m_textures.end()) {
            if (auto cached = tex_it->second.lock()) {
                if (m_cvar_log_texture_ref) {
                    debug::log("Renderer::get_or_load_texture cache hit: '{}' (rc={})",
                               key,
                               cached.use_count());
                }
                return cached;
            }
        }

        const std::filesystem::path full_path = PathUtils::get_assets_root_path() / path;
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

        const uint32_t w = static_cast<uint32_t>(rgba->w);
        const uint32_t h = static_cast<uint32_t>(rgba->h);

        SDL_GPUTextureCreateInfo tci{};
        tci.type = SDL_GPU_TEXTURETYPE_2D;
        tci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        tci.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
        tci.width = w;
        tci.height = h;
        tci.layer_count_or_depth = 1;
        tci.num_levels = 1;
        tci.sample_count = SDL_GPU_SAMPLECOUNT_1;

        SDL_GPUTexture* gpu_tex = SDL_CreateGPUTexture(m_gpu_device, &tci);
        if (!gpu_tex) {
            debug::log_error("SDL_CreateGPUTexture failed: {}", SDL_GetError());
            SDL_DestroySurface(rgba);
            return TextureRef();
        }

        if (!upload_texture_rgba(gpu_tex, rgba->pixels, w, h)) {
            SDL_ReleaseGPUTexture(m_gpu_device, gpu_tex);
            SDL_DestroySurface(rgba);
            return TextureRef();
        }

        SDL_DestroySurface(rgba);

        TextureRef texture(new Texture(*this, gpu_tex, w, h, key));
        m_textures.emplace(key, std::weak_ptr<Texture>(texture));

        if (m_cvar_log_texture_ref) {
            debug::log("Renderer::get_or_load_texture loaded: '{}' (rc=1)", key);
        }

        return texture;
    }

    void Renderer::release_texture(const Texture& texture) {
        if (m_cvar_log_texture_ref) {
            debug::log("Renderer::release_texture: '{}' [destroyed]", texture.m_path);
        }

        m_textures.erase(texture.m_path);
        if (texture.m_gpu_texture) {
            SDL_ReleaseGPUTexture(m_gpu_device, texture.m_gpu_texture);
        }
    }

    ShaderId Renderer::get_or_build_sprite_shader(const std::string& path) {
        if (path.empty()) {
            return DEFAULT_SPRITE_SHADER_ID;
        }

        const std::string key = std::filesystem::path(path).lexically_normal().string();

        auto it = m_shader_path_to_id.find(key);
        if (it != m_shader_path_to_id.end()) {
            return it->second;
        }

        SDL_GPUGraphicsPipeline* pipeline = build_sprite_pipeline(key);
        if (!pipeline) {
            // Alias to default so subsequent lookups are O(1) and silent.
            m_shader_path_to_id.emplace(key, DEFAULT_SPRITE_SHADER_ID);
            return DEFAULT_SPRITE_SHADER_ID;
        }

        const ShaderId id = static_cast<ShaderId>(m_sprite_pipelines.size());
        m_sprite_pipelines.push_back(pipeline);
        m_shader_path_to_id.emplace(key, id);
        return id;
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

    bool Renderer::init_offscreen_target() {
        SDL_GPUTextureCreateInfo tci{};
        tci.type = SDL_GPU_TEXTURETYPE_2D;
        tci.format = m_offscreen_format;
        tci.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        tci.width = m_logical_width;
        tci.height = m_logical_height;
        tci.layer_count_or_depth = 1;
        tci.num_levels = 1;
        tci.sample_count = SDL_GPU_SAMPLECOUNT_1;

        m_offscreen_color = SDL_CreateGPUTexture(m_gpu_device, &tci);
        if (!m_offscreen_color) {
            debug::log_error("SDL_CreateGPUTexture (offscreen) failed: {}", SDL_GetError());
            return false;
        }

        return true;
    }

    bool Renderer::init_samplers() {
        SDL_GPUSamplerCreateInfo sprite_info{};
        sprite_info.min_filter = SDL_GPU_FILTER_LINEAR;
        sprite_info.mag_filter = SDL_GPU_FILTER_NEAREST;
        sprite_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
        sprite_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        sprite_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        sprite_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

        m_sprite_sampler = SDL_CreateGPUSampler(m_gpu_device, &sprite_info);
        if (!m_sprite_sampler) {
            debug::log_error("SDL_CreateGPUSampler (sprite) failed: {}", SDL_GetError());
            return false;
        }

        SDL_GPUSamplerCreateInfo blit_info = sprite_info;
        blit_info.min_filter = SDL_GPU_FILTER_LINEAR;
        blit_info.mag_filter = SDL_GPU_FILTER_LINEAR;

        m_blit_sampler = SDL_CreateGPUSampler(m_gpu_device, &blit_info);
        if (!m_blit_sampler) {
            debug::log_error("SDL_CreateGPUSampler (blit) failed: {}", SDL_GetError());
            return false;
        }

        return true;
    }

    bool Renderer::init_quad_vbo() {
        // 6 vertices for two triangles covering the unit square [0,1] x [0,1].
        // Layout per vertex: float2 pos, float2 uv.
        const float verts[] = {
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
        };

        SDL_GPUBufferCreateInfo bci{};
        bci.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bci.size = sizeof(verts);
        m_quad_vbo = SDL_CreateGPUBuffer(m_gpu_device, &bci);
        if (!m_quad_vbo) {
            debug::log_error("SDL_CreateGPUBuffer (quad) failed: {}", SDL_GetError());
            return false;
        }

        return upload_buffer(m_quad_vbo, verts, sizeof(verts));
    }

    bool Renderer::init_blit_pipeline() {
        const std::filesystem::path shader_dir = PathUtils::get_assets_root_path() / "builtin" / "shaders";

        SDL_GPUShader* vs = load_shader(m_gpu_device,
                                        shader_dir / "blit.vert.hlsl",
                                        SDL_SHADERCROSS_SHADERSTAGE_VERTEX);

        if (!vs) {
            return false;
        }

        SDL_GPUShader* fs = load_shader(m_gpu_device,
                                        shader_dir / "blit.frag.hlsl",
                                        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);

        if (!fs) {
            SDL_ReleaseGPUShader(m_gpu_device, vs);
            return false;
        }

        SDL_GPUColorTargetDescription ctd{};
        ctd.format = SDL_GetGPUSwapchainTextureFormat(m_gpu_device, m_sdl_context.get_window());
        ctd.blend_state.enable_blend = false;

        SDL_GPUGraphicsPipelineCreateInfo gci{};
        gci.vertex_shader = vs;
        gci.fragment_shader = fs;
        // No vertex buffers — blit VS synthesizes verts from SV_VertexID.
        gci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        gci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
        gci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
        gci.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
        gci.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
        gci.target_info.color_target_descriptions = &ctd;
        gci.target_info.num_color_targets = 1;
        gci.target_info.has_depth_stencil_target = false;

        m_blit_pipeline = SDL_CreateGPUGraphicsPipeline(m_gpu_device, &gci);

        SDL_ReleaseGPUShader(m_gpu_device, vs);
        SDL_ReleaseGPUShader(m_gpu_device, fs);

        if (!m_blit_pipeline) {
            debug::log_error("SDL_CreateGPUGraphicsPipeline (blit) failed: {}", SDL_GetError());
            return false;
        }

        return true;
    }

    bool Renderer::init_line_pipeline() {
        const std::filesystem::path shader_dir = PathUtils::get_assets_root_path() / "builtin" / "shaders";

        SDL_GPUShader* vs = load_shader(m_gpu_device,
                                        shader_dir / "line.vert.hlsl",
                                        SDL_SHADERCROSS_SHADERSTAGE_VERTEX);

        if (!vs) {
            return false;
        }

        SDL_GPUShader* fs = load_shader(m_gpu_device,
                                        shader_dir / "line.frag.hlsl",
                                        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);

        if (!fs) {
            SDL_ReleaseGPUShader(m_gpu_device, vs);
            return false;
        }

        // LineVertex layout: float2 pos (offset 0), float4 color (offset 8).
        SDL_GPUVertexBufferDescription vbd{};
        vbd.slot = 0;
        vbd.pitch = sizeof(LineVertex);
        vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

        SDL_GPUVertexAttribute attrs[2]{};
        attrs[0].location = 0;
        attrs[0].buffer_slot = 0;
        attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[0].offset = 0;
        attrs[1].location = 1;
        attrs[1].buffer_slot = 0;
        attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        attrs[1].offset = sizeof(Vector2);

        SDL_GPUColorTargetDescription ctd{};
        ctd.format = m_offscreen_format;
        ctd.blend_state.enable_blend = true;
        ctd.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        ctd.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        ctd.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        ctd.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        ctd.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        ctd.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

        SDL_GPUGraphicsPipelineCreateInfo gci{};
        gci.vertex_shader = vs;
        gci.fragment_shader = fs;
        gci.vertex_input_state.vertex_buffer_descriptions = &vbd;
        gci.vertex_input_state.num_vertex_buffers = 1;
        gci.vertex_input_state.vertex_attributes = attrs;
        gci.vertex_input_state.num_vertex_attributes = 2;
        gci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        gci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
        gci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
        gci.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
        gci.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
        gci.target_info.color_target_descriptions = &ctd;
        gci.target_info.num_color_targets = 1;
        gci.target_info.has_depth_stencil_target = false;

        m_line_pipeline = SDL_CreateGPUGraphicsPipeline(m_gpu_device, &gci);

        SDL_ReleaseGPUShader(m_gpu_device, vs);
        SDL_ReleaseGPUShader(m_gpu_device, fs);

        if (!m_line_pipeline) {
            debug::log_error("SDL_CreateGPUGraphicsPipeline (line) failed: {}", SDL_GetError());
            return false;
        }

        const uint32_t buffer_bytes = MAX_LINE_VERTICES * sizeof(LineVertex);

        SDL_GPUBufferCreateInfo bci{};
        bci.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bci.size = buffer_bytes;
        m_line_vbo = SDL_CreateGPUBuffer(m_gpu_device, &bci);
        if (!m_line_vbo) {
            debug::log_error("SDL_CreateGPUBuffer (line) failed: {}", SDL_GetError());
            return false;
        }

        SDL_GPUTransferBufferCreateInfo tbi{};
        tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbi.size = buffer_bytes;
        m_line_transfer_buffer = SDL_CreateGPUTransferBuffer(m_gpu_device, &tbi);
        if (!m_line_transfer_buffer) {
            debug::log_error("SDL_CreateGPUTransferBuffer (line) failed: {}", SDL_GetError());
            return false;
        }

        return true;
    }

    bool Renderer::init_default_sprite_pipeline() {
        const std::string default_key = std::filesystem::path("builtin/shaders/sprite").lexically_normal().string();
        SDL_GPUGraphicsPipeline* pipeline = build_sprite_pipeline(default_key);
        if (!pipeline) {
            return false;
        }

        // Default lands at slot 0 = DEFAULT_SPRITE_SHADER_ID.
        m_sprite_pipelines.push_back(pipeline);
        m_shader_path_to_id.emplace(default_key, DEFAULT_SPRITE_SHADER_ID);
        return true;
    }

    SDL_GPUGraphicsPipeline* Renderer::build_sprite_pipeline(const std::string& path) {
        const std::filesystem::path assets_root = PathUtils::get_assets_root_path();
        const std::filesystem::path vert_path = assets_root / (path + ".vert.hlsl");
        const std::filesystem::path frag_path = assets_root / (path + ".frag.hlsl");

        SDL_GPUShader* vs = load_shader(m_gpu_device, vert_path, SDL_SHADERCROSS_SHADERSTAGE_VERTEX);
        if (!vs) {
            return nullptr;
        }

        SDL_GPUShader* fs = load_shader(m_gpu_device, frag_path, SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT);
        if (!fs) {
            SDL_ReleaseGPUShader(m_gpu_device, vs);
            return nullptr;
        }

        SDL_GPUVertexBufferDescription vbd{};
        vbd.slot = 0;
        vbd.pitch = 4 * sizeof(float);
        vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vbd.instance_step_rate = 0;

        SDL_GPUVertexAttribute attrs[2]{};
        attrs[0].location = 0;
        attrs[0].buffer_slot = 0;
        attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[0].offset = 0;
        attrs[1].location = 1;
        attrs[1].buffer_slot = 0;
        attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[1].offset = 2 * sizeof(float);

        SDL_GPUColorTargetDescription ctd{};
        ctd.format = m_offscreen_format;
        ctd.blend_state.enable_blend = true;
        ctd.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        ctd.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        ctd.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        ctd.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        ctd.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        ctd.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

        SDL_GPUGraphicsPipelineCreateInfo gci{};
        gci.vertex_shader = vs;
        gci.fragment_shader = fs;
        gci.vertex_input_state.vertex_buffer_descriptions = &vbd;
        gci.vertex_input_state.num_vertex_buffers = 1;
        gci.vertex_input_state.vertex_attributes = attrs;
        gci.vertex_input_state.num_vertex_attributes = 2;
        gci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        gci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
        gci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
        gci.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
        gci.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
        gci.target_info.color_target_descriptions = &ctd;
        gci.target_info.num_color_targets = 1;
        gci.target_info.has_depth_stencil_target = false;

        SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(m_gpu_device, &gci);

        SDL_ReleaseGPUShader(m_gpu_device, vs);
        SDL_ReleaseGPUShader(m_gpu_device, fs);

        if (!pipeline) {
            debug::log_error("SDL_CreateGPUGraphicsPipeline (sprite '{}') failed: {}", path, SDL_GetError());
            return nullptr;
        }

        return pipeline;
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
        SDL_PushGPUFragmentUniformData(m_command_buffer, 0, &fsu, sizeof(fsu));

        SDL_GPUTextureSamplerBinding ts{};
        ts.texture = sp.texture->m_gpu_texture;
        ts.sampler = m_sprite_sampler;
        SDL_BindGPUFragmentSamplers(pass, 0, &ts, 1);

        SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
    }

    bool Renderer::upload_buffer(SDL_GPUBuffer* dst_buffer, const void* data, uint32_t size) {
        SDL_GPUTransferBufferCreateInfo tbi{};
        tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbi.size = size;
        SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(m_gpu_device, &tbi);
        if (!tb) {
            debug::log_error("SDL_CreateGPUTransferBuffer failed: {}", SDL_GetError());
            return false;
        }

        void* map = SDL_MapGPUTransferBuffer(m_gpu_device, tb, false);
        if (!map) {
            debug::log_error("SDL_MapGPUTransferBuffer failed: {}", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(m_gpu_device, tb);
            return false;
        }
        std::memcpy(map, data, size);
        SDL_UnmapGPUTransferBuffer(m_gpu_device, tb);

        SDL_GPUCommandBuffer* upload_cmd = SDL_AcquireGPUCommandBuffer(m_gpu_device);
        SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(upload_cmd);

        SDL_GPUTransferBufferLocation src{};
        src.transfer_buffer = tb;
        src.offset = 0;
        SDL_GPUBufferRegion dst{};
        dst.buffer = dst_buffer;
        dst.offset = 0;
        dst.size = size;
        SDL_UploadToGPUBuffer(copy, &src, &dst, false);

        SDL_EndGPUCopyPass(copy);

        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(upload_cmd);
        if (fence) {
            SDL_WaitForGPUFences(m_gpu_device, true, &fence, 1);
            SDL_ReleaseGPUFence(m_gpu_device, fence);
        }
        SDL_ReleaseGPUTransferBuffer(m_gpu_device, tb);
        return true;
    }

    bool Renderer::upload_texture_rgba(SDL_GPUTexture* dst_texture,
                                       const void* pixels,
                                       uint32_t width,
                                       uint32_t height) {
        const uint32_t size = width * height * 4;

        SDL_GPUTransferBufferCreateInfo tbi{};
        tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tbi.size = size;
        SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(m_gpu_device, &tbi);
        if (!tb) {
            debug::log_error("SDL_CreateGPUTransferBuffer (texture) failed: {}", SDL_GetError());
            return false;
        }

        void* map = SDL_MapGPUTransferBuffer(m_gpu_device, tb, false);
        if (!map) {
            debug::log_error("SDL_MapGPUTransferBuffer (texture) failed: {}", SDL_GetError());
            SDL_ReleaseGPUTransferBuffer(m_gpu_device, tb);
            return false;
        }
        std::memcpy(map, pixels, size);
        SDL_UnmapGPUTransferBuffer(m_gpu_device, tb);

        SDL_GPUCommandBuffer* upload_cmd = SDL_AcquireGPUCommandBuffer(m_gpu_device);
        SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(upload_cmd);

        SDL_GPUTextureTransferInfo src{};
        src.transfer_buffer = tb;
        src.offset = 0;
        src.pixels_per_row = width;
        src.rows_per_layer = height;

        SDL_GPUTextureRegion dst{};
        dst.texture = dst_texture;
        dst.mip_level = 0;
        dst.layer = 0;
        dst.x = 0;
        dst.y = 0;
        dst.z = 0;
        dst.w = width;
        dst.h = height;
        dst.d = 1;

        SDL_UploadToGPUTexture(copy, &src, &dst, false);

        SDL_EndGPUCopyPass(copy);

        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(upload_cmd);
        if (fence) {
            SDL_WaitForGPUFences(m_gpu_device, true, &fence, 1);
            SDL_ReleaseGPUFence(m_gpu_device, fence);
        }
        SDL_ReleaseGPUTransferBuffer(m_gpu_device, tb);
        return true;
    }

    void Renderer::debug_texture_refs() {
        if (!m_cvar_show_texture_refs) {
            return;
        }

        if (ImGui::Begin("Texture Refs")) {
            // Count per-texture refs held by pending sprite queues. Each draw_sprite
            // call copies the TextureRef into the pending vector for the duration of
            // the frame, which inflates use_count() but is not a "logical" holder.
            std::unordered_map<const Texture*, int> pending_refs;
            for (const auto& sp : m_pending_sprites) {
                if (sp.texture) {
                    pending_refs[sp.texture.get()] += 1;
                }
            }
            for (const auto& sp : m_pending_overlay_sprites) {
                if (sp.texture) {
                    pending_refs[sp.texture.get()] += 1;
                }
            }

            int total_game = 0;
            int total_all = 0;
            for (const auto& [path, weak] : m_textures) {
                if (auto tex = weak.lock()) {
                    // Subtract 1 because `tex` itself is a strong ref held only for this iteration.
                    const int all = static_cast<int>(tex.use_count()) - 1;
                    const auto pit = pending_refs.find(tex.get());
                    const int pending = pit != pending_refs.end() ? pit->second : 0;
                    total_all += all;
                    total_game += all - pending;
                }
            }
            ImGui::Text("Textures: %zu | Game refs: %d | All refs: %d",
                        m_textures.size(),
                        total_game,
                        total_all);

            const ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                          ImGuiTableFlags_RowBg |
                                          ImGuiTableFlags_ScrollY;

            if (ImGui::BeginTable("texture_refs", 4, flags)) {
                ImGui::TableSetupColumn("size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("game refs", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("all refs", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& [path, weak] : m_textures) {
                    auto tex = weak.lock();
                    if (!tex) {
                        continue;
                    }
                    const int all = static_cast<int>(tex.use_count()) - 1;
                    const auto pit = pending_refs.find(tex.get());
                    const int pending = pit != pending_refs.end() ? pit->second : 0;
                    const int game = all - pending;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%ux%u", tex->get_width(), tex->get_height());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", game);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", all);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(tex->get_path().c_str());
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void Renderer::debug_sprite_queue() {
        if (m_cvar_log_sprite_queue) {
            debug::log("Renderer sprite order ({} sprites):", m_pending_sprites.size());
            for (size_t i = 0; i < m_pending_sprites.size(); ++i) {
                const Sprite& sp = m_pending_sprites[i];
                const char* tex_path = sp.texture ? sp.texture->get_path().c_str() : "<unknown>";
                debug::log("  [{}] z={} shader={} tex={}", i, sp.z_index, sp.material.shader_id, tex_path);
            }
        }

        if (m_cvar_show_sprite_queue) {
            if (ImGui::Begin("Sprite Queue")) {
                ImGui::Text("Total: %zu", m_pending_sprites.size());
                const int columns = 4;
                const ImGuiTabBarFlags flags = ImGuiTableFlags_Borders |
                                               ImGuiTableFlags_RowBg |
                                               ImGuiTableFlags_ScrollY;

                if (ImGui::BeginTable("queue", columns, flags)) {
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                    ImGui::TableSetupColumn("z_index", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("shader_id", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("texture", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < m_pending_sprites.size(); ++i) {
                        const Sprite& sp = m_pending_sprites[i];
                        const char* tex_path = sp.texture ? sp.texture->get_path().c_str() : "<unknown>";
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%zu", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", sp.z_index);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%d", sp.material.shader_id);
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(tex_path);
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::End();
        }
    }

    void Renderer::register_cvars(Console& console) {
        console.register_cvar("r_log_texture_ref",
                              "Log every texture load/unload/cache-hit",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_log_texture_ref = cvar.bool_value();
                              });

        console.register_cvar("r_show_texture_refs",
                              "Show a texture cache window (id, size, ref_count, path)",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_show_texture_refs = cvar.bool_value();
                              });

        console.register_cvar("r_log_sprite_queue",
                              "Log sprite queue (z_index, shader_id, texture) each frame",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_log_sprite_queue = cvar.bool_value();
                              });

        console.register_cvar("r_show_sprite_queue",
                              "Show a sprite queue window (z_index, shader_id, texture)",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_show_sprite_queue = cvar.bool_value();
                              });
    }
}
