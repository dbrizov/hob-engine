#include "assets.h"

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

        return texture_id;
    }

    bool Assets::unload_texture(TextureId id) {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            Renderer::destroy_texture(it->second);
            m_textures.erase(it);
            m_texture_widths.erase(id);
            m_texture_heights.erase(id);
            return true;
        }

        return false;
    }

    void Assets::get_texture_size(TextureId id, int& out_width, int& out_height) const {
        auto width_it = m_texture_widths.find(id);
        auto height_it = m_texture_heights.find(id);
        out_width = (width_it != m_texture_widths.end()) ? width_it->second : 0;
        out_height = (height_it != m_texture_heights.end()) ? height_it->second : 0;
    }

    void Assets::unload_all_textures() {
        for (auto& [id, handle] : m_textures) {
            Renderer::destroy_texture(handle);
        }

        m_textures.clear();
        m_texture_widths.clear();
        m_texture_heights.clear();
        debug::log("Assets::unload_all_textures()");
    }
}
