#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace hob {
    using GlTextureHandle = uint32_t;
    using TextureId = int32_t;
    constexpr TextureId INVALID_TEXTURE_ID = -1;

    class Assets {
        mutable TextureId m_next_texture_id;
        mutable std::unordered_map<TextureId, GlTextureHandle> m_textures;
        mutable std::unordered_map<TextureId, int> m_texture_widths;
        mutable std::unordered_map<TextureId, int> m_texture_heights;
        mutable std::unordered_map<std::string, TextureId> m_path_to_id;
        mutable std::unordered_map<TextureId, std::string> m_id_to_path;
        mutable std::unordered_map<TextureId, int> m_ref_counts;

    public:
        Assets();
        ~Assets();

        GlTextureHandle get_texture(TextureId id) const;
        TextureId load_texture(const std::filesystem::path& path);
        bool unload_texture(TextureId id);

        void get_texture_size(TextureId id, int& out_width, int& out_height) const;

    private:
        void unload_all_textures();
    };
}
