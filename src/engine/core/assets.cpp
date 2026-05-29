#include "assets.h"

#include <cassert>

#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>

#include "logging.h"
#include "renderer.h"

namespace hob {
    Assets::Assets()
        : m_next_texture_id(0)
        , m_textures() {
    }

    Assets::~Assets() {
        unload_all_textures();
    }

    GlTextureHandle Assets::get_texture(TextureId id) const {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            return it->second;
        }

        debug::log_error("Assets::get_texture failed. Invalid texture id: {}", id);
        return 0;
    }

    TextureId Assets::load_texture(const std::filesystem::path& path) {
        std::error_code ec;
        std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
        if (ec) {
            canonical = path.lexically_normal();
        }
        const std::string key = canonical.string();

        auto existing = m_path_to_id.find(key);
        if (existing != m_path_to_id.end()) {
            const TextureId id = existing->second;
            const int refcount = ++m_ref_counts[id];
            debug::log("Assets::load_texture cache hit: {} id={} refcount={}", key, id, refcount);
            return id;
        }

        SDL_Surface* surface = IMG_Load(path.string().c_str());
        if (!surface) {
            debug::log_error("IMG_Load failed: {}", SDL_GetError());
            return INVALID_TEXTURE_ID;
        }

        SDL_Surface* rgba = surface;
        if (surface->format != SDL_PIXELFORMAT_RGBA32) {
            rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(surface);
            if (!rgba) {
                debug::log_error("SDL_ConvertSurface failed: {}", SDL_GetError());
                return INVALID_TEXTURE_ID;
            }
        }

        GlTextureHandle handle = Renderer::create_texture_from_pixels(rgba->pixels, rgba->w, rgba->h);
        const int w = rgba->w;
        const int h = rgba->h;
        SDL_DestroySurface(rgba);

        TextureId texture_id = m_next_texture_id;
        m_next_texture_id += 1;

        m_textures.emplace(texture_id, handle);
        m_texture_widths.emplace(texture_id, w);
        m_texture_heights.emplace(texture_id, h);
        m_path_to_id.emplace(key, texture_id);
        m_id_to_path.emplace(texture_id, key);
        m_ref_counts.emplace(texture_id, 1);

        debug::log("Assets::load_texture loaded: {} id={} refcount=1", key, texture_id);
        return texture_id;
    }

    bool Assets::unload_texture(TextureId id) {
        auto ref_it = m_ref_counts.find(id);
        if (ref_it == m_ref_counts.end()) {
            debug::log_error("Assets::unload_texture: unknown id={}", id);
            return false;
        }

        ref_it->second -= 1;
        const int refcount = ref_it->second;

        auto path_it = m_id_to_path.find(id);
        const std::string path = (path_it != m_id_to_path.end()) ? path_it->second : std::string("?");

        if (refcount > 0) {
            debug::log("Assets::unload_texture: {} id={} refcount={}", path, id, refcount);
            return true;
        }

        debug::log("Assets::unload_texture: {} id={} refcount=0 (destroyed)", path, id);

        auto tex_it = m_textures.find(id);
        assert(tex_it != m_textures.end() && "Texture not found.");
        Renderer::destroy_texture(tex_it->second);
        m_textures.erase(tex_it);

        m_texture_widths.erase(id);
        m_texture_heights.erase(id);

        if (path_it != m_id_to_path.end()) {
            m_path_to_id.erase(path_it->second);
            m_id_to_path.erase(path_it);
        }

        m_ref_counts.erase(ref_it);
        return true;
    }

    void Assets::get_texture_size(TextureId id, int& out_width, int& out_height) const {
        auto width_it = m_texture_widths.find(id);
        auto height_it = m_texture_heights.find(id);
        out_width = (width_it != m_texture_widths.end()) ? width_it->second : 0;
        out_height = (height_it != m_texture_heights.end()) ? height_it->second : 0;
    }

    void Assets::unload_all_textures() {
        for (auto& [id, handle] : m_textures) {
            auto path_it = m_id_to_path.find(id);
            const std::string path = (path_it != m_id_to_path.end()) ? path_it->second : std::string("?");
            auto ref_it = m_ref_counts.find(id);
            const int refcount = (ref_it != m_ref_counts.end()) ? ref_it->second : -1;
            debug::log_error("Assets: leaked texture {} id={} refcount={}", path, id, refcount);
            Renderer::destroy_texture(handle);
        }

        m_textures.clear();
        m_texture_widths.clear();
        m_texture_heights.clear();
        m_path_to_id.clear();
        m_id_to_path.clear();
        m_ref_counts.clear();
    }
}
