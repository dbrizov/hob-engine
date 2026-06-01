#include "renderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <unordered_set>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "engine/core/engine_config.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
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

    // TextureRef
    TextureRef::TextureRef(Renderer& renderer, TextureId id, uint32_t width, uint32_t height)
        : m_renderer(&renderer)
        , m_id(id)
        , m_width(width)
        , m_height(height) {
    }

    TextureRef::~TextureRef() {
        reset();
    }

    TextureRef::TextureRef(TextureRef&& other) noexcept
        : m_renderer(other.m_renderer)
        , m_id(other.m_id)
        , m_width(other.m_width)
        , m_height(other.m_height) {
        other.m_renderer = nullptr;
        other.m_id = INVALID_TEXTURE_ID;
        other.m_width = 0;
        other.m_height = 0;
    }

    TextureRef& TextureRef::operator=(TextureRef&& other) noexcept {
        if (this != &other) {
            reset();

            m_renderer = other.m_renderer;
            m_id = other.m_id;
            m_width = other.m_width;
            m_height = other.m_height;

            other.m_renderer = nullptr;
            other.m_id = INVALID_TEXTURE_ID;
            other.m_width = 0;
            other.m_height = 0;
        }

        return *this;
    }

    void TextureRef::reset() {
        if (m_renderer != nullptr) {
            m_renderer->unload_texture(m_id);
            m_renderer = nullptr;
            m_id = INVALID_TEXTURE_ID;
            m_width = 0;
            m_height = 0;
        }
    }

    bool TextureRef::is_valid() const {
        return m_renderer != nullptr;
    }

    TextureId TextureRef::get_id() const {
        return m_id;
    }

    uint32_t TextureRef::get_width() const {
        return m_width;
    }

    uint32_t TextureRef::get_height() const {
        return m_height;
    }

    // Renderer
    Renderer::Renderer(const EngineConfig& config, const SdlContext& sdl_context, Console& console)
        : m_sdl_context(sdl_context)
        , m_gpu_device(sdl_context.get_gpu_device())
        , m_logical_width(config.graphics_config.logical_resolution_width)
        , m_logical_height(config.graphics_config.logical_resolution_height)
        , m_pixels_per_meter(config.graphics_config.pixels_per_meter)
        , m_projection(ortho_top_left(static_cast<float>(m_logical_width),
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

        if (!init_offscreen_target()) {
            return;
        }

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
        // Release all cached textures first.
        for (auto& [id, entry] : m_textures) {
            if (entry.texture) {
                SDL_ReleaseGPUTexture(m_gpu_device, entry.texture);
            }
        }
        m_textures.clear();
        m_path_to_id.clear();

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

    uint32_t Renderer::get_pixels_per_meter() const {
        return m_pixels_per_meter;
    }

    float Renderer::get_pixels_per_meter_f() const {
        return static_cast<float>(m_pixels_per_meter);
    }

    void Renderer::render_sprite(TextureId texture_id,
                                 ShaderId shader_id,
                                 const Vector2& screen_pos,
                                 const Vector2& size_pixels,
                                 const Vector2& pivot_pixel,
                                 float rotation_rad,
                                 const Color& tint) {
        const ShaderId resolved_shader_id =
            (shader_id == INVALID_SHADER_ID) ? DEFAULT_SPRITE_SHADER_ID : shader_id;
        m_pending_sprites.push_back(
            {texture_id, resolved_shader_id, screen_pos, size_pixels, pivot_pixel, rotation_rad, tint});
    }

    void Renderer::render_line(const Vector2& a, const Vector2& b, const Color& color, float thickness) {
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
        const std::filesystem::path full_path = PathUtils::get_assets_root_path() / path;
        const std::string key = full_path.lexically_normal().string();

        auto it = m_path_to_id.find(key);
        if (it != m_path_to_id.end()) {
            TextureEntry& entry = m_textures.at(it->second);
            entry.ref_count += 1;

            if (m_cvar_log_textures) {
                debug::log("Renderer::get_or_load_texture cache hit: '{}' (id={}, rc={})",
                           key,
                           it->second,
                           entry.ref_count);
            }

            return TextureRef(*this, it->second, entry.width, entry.height);
        }

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

        const TextureId texture_id = m_next_texture_id++;
        m_textures.emplace(texture_id, TextureEntry{gpu_tex, w, h, key, 1});
        m_path_to_id.emplace(key, texture_id);

        if (m_cvar_log_textures) {
            debug::log("Renderer::get_or_load_texture loaded: '{}' (id={}, rc=1)", key, texture_id);
        }

        return TextureRef(*this, texture_id, w, h);
    }

    bool Renderer::unload_texture(TextureId id) {
        auto it = m_textures.find(id);
        if (it == m_textures.end()) {
            debug::log_error("Renderer::unload_texture: 'unknown' (id={})", id);
            return false;
        }

        TextureEntry& entry = it->second;
        entry.ref_count -= 1;

        if (entry.ref_count > 0) {
            if (m_cvar_log_textures) {
                debug::log("Renderer::unload_texture: '{}' (id={}, rc={})", entry.path, id, entry.ref_count);
            }
            return true;
        }

        if (m_cvar_log_textures) {
            debug::log("Renderer::unload_texture: '{}' (id={}, rc=0) [destroyed]", entry.path, id);
        }

        m_path_to_id.erase(entry.path);
        if (entry.texture) {
            SDL_ReleaseGPUTexture(m_gpu_device, entry.texture);
        }
        m_textures.erase(it);
        return true;
    }

    void Renderer::record_world(SDL_GPUCommandBuffer* cmd) {
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
        ct.clear_color = CLEAR_COLOR;
        ct.load_op = SDL_GPU_LOADOP_CLEAR;
        ct.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &ct, 1, nullptr);
        if (!pass) {
            m_pending_sprites.clear();
            m_pending_lines.clear();
            return;
        }

        if (!m_pending_sprites.empty()) {
            // TODO: this sort breaks z_index ordering across different shaders — a low-z
            // sprite using shader B will draw after a high-z sprite using shader A. Fix
            // once Sprite carries z_index (sort by (shader_id, z_index) or similar).
            std::stable_sort(m_pending_sprites.begin(), m_pending_sprites.end(),
                             [](const Sprite& a, const Sprite& b) {
                                 return a.shader_id < b.shader_id;
                             });

            SDL_GPUBufferBinding vb{};
            vb.buffer = m_quad_vbo;
            vb.offset = 0;
            SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

            ShaderId bound_shader = INVALID_SHADER_ID;

            for (const Sprite& sp : m_pending_sprites) {
                auto it = m_textures.find(sp.texture_id);
                if (it == m_textures.end() || !it->second.texture) {
                    continue;
                }

                if (sp.shader_id != bound_shader) {
                    SDL_BindGPUGraphicsPipeline(pass, m_sprite_pipelines[sp.shader_id]);
                    bound_shader = sp.shader_id;
                }

                SpriteVSUniforms vsu{};
                std::memcpy(vsu.proj, m_projection.data(), sizeof(vsu.proj));
                vsu.screen_pos[0] = sp.screen_pos.x;
                vsu.screen_pos[1] = sp.screen_pos.y;
                vsu.size[0] = sp.size_pixels.x;
                vsu.size[1] = sp.size_pixels.y;
                vsu.pivot[0] = sp.pivot_pixel.x;
                vsu.pivot[1] = sp.pivot_pixel.y;
                // World rotation is y-up CCW; logical screen is y-down. Negate so positive
                // world rotation remains visually CCW.
                vsu.rotation = -sp.rotation_rad;
                SDL_PushGPUVertexUniformData(cmd, 0, &vsu, sizeof(vsu));

                SDL_PushGPUFragmentUniformData(cmd, 0, &sp.tint, sizeof(Color));

                SDL_GPUTextureSamplerBinding ts{};
                ts.texture = it->second.texture;
                ts.sampler = m_sprite_sampler;
                SDL_BindGPUFragmentSamplers(pass, 0, &ts, 1);

                SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
            }
        }

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

        m_pending_sprites.clear();
        m_pending_lines.clear();
    }

    void Renderer::record_blit(SDL_GPURenderPass* swap_pass) {
        SDL_BindGPUGraphicsPipeline(swap_pass, m_blit_pipeline);

        SDL_GPUTextureSamplerBinding ts{};
        ts.texture = m_offscreen_color;
        ts.sampler = m_blit_sampler;
        SDL_BindGPUFragmentSamplers(swap_pass, 0, &ts, 1);

        SDL_DrawGPUPrimitives(swap_pass, 3, 1, 0, 0);
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

    void Renderer::register_cvars(Console& console) {
        console.register_cvar("rend_log_textures",
                              "Log every texture load/unload/cache-hit",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_log_textures = cvar.bool_value();
                              });
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
}
