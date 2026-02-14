#include "Assets.h"

#include <SDL_image.h>
#include <SDL_render.h>
#include <cassert>
#include <fmt/base.h>

Assets::Assets(SDL_Renderer* renderer)
    : m_renderer(renderer)
      , m_next_texture_id(0)
      , m_white_pixel_texture_id(INVALID_TEXTURE_ID)
      , m_textures() {
}

Assets::~Assets() {
    unload_all_textures();
}

SDL_Texture* Assets::get_texture(TextureId id) const {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        return it->second;
    }

    fmt::println(stderr, "Assets::get_texture failed. Invalid texture id: {}", id);
    return nullptr;
}

SDL_Texture* Assets::get_white_pixel_texture() const {
    if (m_white_pixel_texture_id == INVALID_TEXTURE_ID) {
        SDL_Texture* white_pixel_texture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STATIC,
            1, 1);

        assert(white_pixel_texture != nullptr && "Could not create SDL texture");

        uint32_t pixel = 0xFFFFFFFF; // White RGBA
        SDL_UpdateTexture(white_pixel_texture, nullptr, &pixel, sizeof(pixel));

        m_white_pixel_texture_id = m_next_texture_id;
        m_next_texture_id += 1;

        m_textures.emplace(m_white_pixel_texture_id, white_pixel_texture);
    }

    return get_texture(m_white_pixel_texture_id);
}

TextureId Assets::load_texture(const std::filesystem::path& path) {
    SDL_Texture* texture = IMG_LoadTexture(m_renderer, path.string().c_str());
    if (!texture) {
        fmt::println(stderr, "IMG_LoadTexture failed: {}", IMG_GetError());
        return INVALID_TEXTURE_ID;
    }

    TextureId texture_id = m_next_texture_id;
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
