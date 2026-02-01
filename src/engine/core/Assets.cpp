#include "Assets.h"

#include <SDL_image.h>
#include <SDL_render.h>
#include <fmt/base.h>

Assets::Assets(const std::filesystem::path& assets_root_path, SDL_Renderer* renderer)
    : m_assets_root_path(assets_root_path)
      , m_renderer(renderer)
      , m_textures()
      , m_next_texture_id(INVALID_TEXTURE_ID) {
}

Assets::~Assets() {
    unload_all_textures();
}

const std::filesystem::path& Assets::get_assets_root_path() const {
    return m_assets_root_path;
}

SDL_Texture* Assets::get_texture(TextureId id) const {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        return it->second;
    }

    fmt::println(stderr, "Assets::get_texture failed. Invalid texture id: {}", id);
    return nullptr;
}

TextureId Assets::load_texture(const std::filesystem::path& path) {
    SDL_Texture* texture = IMG_LoadTexture(m_renderer, path.string().c_str());
    if (!texture) {
        fmt::println(stderr, "IMG_LoadTexture failed: {}", IMG_GetError());
        return INVALID_TEXTURE_ID;
    }

    const TextureId texture_id = m_next_texture_id;
    m_next_texture_id += 1;

    m_textures.emplace(texture_id, texture);

    return texture_id;
}

bool Assets::unload_texture(TextureId id) {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        SDL_Texture* texture = it->second;
        SDL_DestroyTexture(texture);
        m_textures.erase(it);

        return true;
    }

    return false;
}

void Assets::unload_all_textures() {
    for (auto& [id, texture] : m_textures) {
        SDL_DestroyTexture(texture);
    }

    m_textures.clear();
    fmt::println("Assets::unload_all_textures()");
}
