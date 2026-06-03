#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace hob {
    struct LuaPathSchemaInfo {
        std::string registry_name; // Lua-side registry table, e.g. "Textures"
        std::string define_name; // Lua-side define entry point, e.g. "DefineTexture"
        std::string type_label; // Used in error messages, e.g. "Texture"
    };

    class LuaPathSchemaRegistry {
        std::vector<LuaPathSchemaInfo> m_schemas;

    public:
        void add_schema(LuaPathSchemaInfo info);

        bool write_to_file(const std::filesystem::path& path) const;
        bool write_meta_to_file(const std::filesystem::path& path) const;
    };

    inline void bind_path_schema(LuaPathSchemaRegistry& schemas,
                                 const char* registry_name,
                                 const char* define_name,
                                 const char* type_label) {
        schemas.add_schema({registry_name, define_name, type_label});
    }
}
