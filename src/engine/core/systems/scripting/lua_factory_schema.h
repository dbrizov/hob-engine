#pragma once

#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace hob {
    enum class LuaFieldResolve {
        Passthrough,
        Asset,
        AssetList,
    };

    struct LuaFactoryFieldInfo {
        std::string name;
        LuaFieldResolve resolve;
    };

    struct LuaFactorySchemaInfo {
        std::string registry_name; // Lua-side registry table, e.g. "Materials"
        std::string define_name; // Lua-side define entry point, e.g. "DefineMaterial"
        std::string lua_type; // Bound usertype name invoked as a factory, e.g. "Material"
        std::vector<LuaFactoryFieldInfo> fields;
    };

    class LuaFactorySchemaRegistry {
        std::vector<LuaFactorySchemaInfo> m_schemas;

    public:
        void add_schema(LuaFactorySchemaInfo info);

        bool write_to_file(const std::filesystem::path& path) const;
    };

    // Records that a usertype bound with a `factory_ctor` accepting a single config table
    // is authorable via a Lua-side `DefineX.Name = { ... }` registry. The generic applier in
    // scripts/engine/factory_def.lua reads the generated schema and synthesizes the entire
    // material_def.lua / animation_clip_def.lua boilerplate.
    //
    // The corresponding bind_usertype<T>(...).factory_ctor(...) call must already exist; this
    // function only records metadata, it does not bind any C++.
    inline void bind_factory_schema(LuaFactorySchemaRegistry& schemas,
                                    const char* registry_name,
                                    const char* define_name,
                                    const char* lua_type,
                                    std::initializer_list<std::pair<const char*, LuaFieldResolve>> fields) {
        LuaFactorySchemaInfo info;
        info.registry_name = registry_name;
        info.define_name = define_name;
        info.lua_type = lua_type;
        info.fields.reserve(fields.size());
        for (const auto& f : fields) {
            info.fields.push_back({f.first, f.second});
        }

        schemas.add_schema(std::move(info));
    }
}
