#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace hob {
    class App;

    using GlTextureHandle = uint32_t;
    using TextureId = int32_t;
    constexpr TextureId INVALID_TEXTURE_ID = -1;

    struct TextureEntry {
        GlTextureHandle handle;
        int width;
        int height;
        std::string path;
        int ref_count;

        TextureEntry(GlTextureHandle _handle, int _width, int _height, std::string _path, int _ref_count) {
            handle = _handle;
            width = _width;
            height = _height;
            path = std::move(_path);
            ref_count = _ref_count;
        }
    };

    class Assets {
        App& m_app;
        TextureId m_next_texture_id = 0;
        std::unordered_map<TextureId, TextureEntry> m_textures;
        std::unordered_map<std::string, TextureId> m_path_to_id;

        bool m_cvar_verbose = false;

    public:
        explicit Assets(App& app);
        ~Assets();

        GlTextureHandle get_texture(TextureId id) const;
        TextureId load_texture(const std::filesystem::path& full_path);
        bool unload_texture(TextureId id);

        void get_texture_size(TextureId id, int& out_width, int& out_height) const;

    private:
        void unload_all_textures();
    };
}
