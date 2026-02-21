#ifndef HOB_ENGINE_ASSETS_H
#define HOB_ENGINE_ASSETS_H
#include <filesystem>
#include <unordered_map>


struct SDL_Renderer;
struct SDL_Texture;

namespace hob {
    using TextureId = int32_t;
    constexpr TextureId INVALID_TEXTURE_ID = -1;

    class Assets {
        SDL_Renderer* m_renderer;
        mutable TextureId m_next_texture_id;
        mutable TextureId m_white_pixel_texture_id;
        mutable std::unordered_map<TextureId, SDL_Texture*> m_textures;

    public:
        explicit Assets(SDL_Renderer* renderer);
        ~Assets();

        SDL_Texture* get_texture(TextureId id) const;
        SDL_Texture* get_white_pixel_texture() const;
        TextureId load_texture(const std::filesystem::path& path);
        bool unload_texture(TextureId id);

    private:
        void unload_all_textures();
    };
}


#endif //HOB_ENGINE_ASSETS_H
