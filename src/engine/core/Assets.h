#ifndef CPP_PLATFORMER_ASSETS_H
#define CPP_PLATFORMER_ASSETS_H
#include <filesystem>
#include <unordered_map>


struct SDL_Renderer;
struct SDL_Texture;

using TextureId = int32_t;
constexpr TextureId INVALID_TEXTURE_ID = -1;


class Assets {
    std::filesystem::path m_assets_root_path;
    std::unordered_map<TextureId, SDL_Texture*> m_textures;
    TextureId m_next_texture_id;

public:
    explicit Assets(const std::filesystem::path& assets_root_path);
    ~Assets();

    const std::filesystem::path& get_assets_root_path() const;
    SDL_Texture* get_texture(TextureId id) const;
    TextureId load_texture(SDL_Renderer* renderer, const std::filesystem::path& path);
    bool unload_texture(TextureId id);

public:
    void unload_all_textures();
};


#endif //CPP_PLATFORMER_ASSETS_H
