#ifndef CPP_PLATFORMER_ASSETS_H
#define CPP_PLATFORMER_ASSETS_H
#include <filesystem>
#include <unordered_map>


struct SDL_Renderer;
struct SDL_Texture;

using TextureId = int32_t;
constexpr TextureId INVALID_TEXTURE_ID = -1;


class Assets {
    SDL_Renderer* m_renderer;
    std::unordered_map<TextureId, SDL_Texture*> m_textures;
    TextureId m_next_texture_id;

public:
    explicit Assets(SDL_Renderer* renderer);
    ~Assets();

    SDL_Texture* get_texture(TextureId id) const;
    TextureId load_texture(const std::filesystem::path& path);
    bool unload_texture(TextureId id);

private:
    void unload_all_textures();
};


#endif //CPP_PLATFORMER_ASSETS_H
