#include "assets.h"

#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>

#include "app.h"
#include "console.h"
#include "logging.h"
#include "renderer.h"

namespace hob {
    Assets::Assets(App& app)
        : m_app(app) {
        m_app.get_console().register_cvar("assets_verbose",
                                          "Log every texture load/unload/cache-hit",
                                          "0",
                                          ConsoleVariableType::Bool,
                                          ConsoleVariableFlags::None,
                                          [this](const ConsoleVariable& cvar) {
                                              m_cvar_verbose = cvar.bool_value();
                                          });
    }

    Assets::~Assets() {
        unload_all_textures();
    }

    GlTextureHandle Assets::get_texture(TextureId id) const {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            return it->second.handle;
        }

        debug::log_error("Assets::get_texture failed. Invalid texture id: {}", id);
        return 0;
    }

    TextureId Assets::load_texture(const std::filesystem::path& full_path) {
        std::error_code error_code;
        std::filesystem::path canonical = std::filesystem::weakly_canonical(full_path, error_code);
        if (error_code) {
            canonical = full_path.lexically_normal();
        }
        const std::string key = canonical.string();

        auto it = m_path_to_id.find(key);
        if (it != m_path_to_id.end()) {
            TextureEntry& entry = m_textures.at(it->second);
            entry.ref_count += 1;

            if (m_cvar_verbose) {
                debug::log("Assets::load_texture cache hit: '{}' (id={}, rc={})", key, it->second, entry.ref_count);
            }

            return it->second;
        }

        SDL_Surface* surface = IMG_Load(full_path.string().c_str());
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

        const GlTextureHandle handle = Renderer::create_texture_from_pixels(rgba->pixels, rgba->w, rgba->h);
        const int w = rgba->w;
        const int h = rgba->h;
        const int ref_count = 1;
        SDL_DestroySurface(rgba);

        const TextureId texture_id = m_next_texture_id;
        m_next_texture_id += 1;

        m_textures.emplace(texture_id, TextureEntry(handle, w, h, key, ref_count));
        m_path_to_id.emplace(key, texture_id);

        if (m_cvar_verbose) {
            debug::log("Assets::load_texture loaded: '{}' (id={}, rc={})", key, texture_id, ref_count);
        }

        return texture_id;
    }

    bool Assets::unload_texture(TextureId id) {
        auto it = m_textures.find(id);
        if (it == m_textures.end()) {
            debug::log_error("Assets::unload_texture: 'unknown' (id={})", id);
            return false;
        }

        TextureEntry& entry = it->second;
        entry.ref_count -= 1;

        if (entry.ref_count > 0) {
            if (m_cvar_verbose) {
                debug::log("Assets::unload_texture: '{}' (id={}, rc={})", entry.path, id, entry.ref_count);
            }

            return true;
        }

        if (m_cvar_verbose) {
            debug::log("Assets::unload_texture: '{}' (id={}, rc={}) [destroyed]", entry.path, id, entry.ref_count);
        }

        Renderer::destroy_texture(entry.handle);
        m_path_to_id.erase(entry.path);
        m_textures.erase(it);
        return true;
    }

    void Assets::get_texture_size(TextureId id, int& out_width, int& out_height) const {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            out_width = it->second.width;
            out_height = it->second.height;
        }
        else {
            out_width = 0;
            out_height = 0;
        }
    }

    void Assets::unload_all_textures() {
        for (auto& [id, entry] : m_textures) {
            debug::log_error("Assets: leaked texture '{}' (id={}, rc={})", entry.path, id, entry.ref_count);
            Renderer::destroy_texture(entry.handle);
        }

        m_textures.clear();
        m_path_to_id.clear();
    }
}
