#pragma once

#include <filesystem>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <sol/sol.hpp>

#include "lua_meta.h"
#include "lua_type_names.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    class Entity;
    class EntitySpawner;
    struct EntityHandle;

    struct LuaComponentSchemaInfo {
        std::string key; // prefab section key, e.g. "rigidbody"
        std::string add_method; // Entity method, e.g. "add_rigidbody"
        std::vector<std::pair<std::string, std::string>> setters; // {field, set_method}
    };

    class LuaComponentSchemaRegistry {
        std::vector<LuaComponentSchemaInfo> m_schemas;

    public:
        void add_schema(std::string key,
                        std::string add_method,
                        std::vector<std::pair<std::string, std::string>> setters);

        bool write_to_file(const std::filesystem::path& path) const;
    };

    // Registers a C++ component type as authorable from a Lua prefab. One call
    // does three things keyed by a single `add_method` string:
    //   1. Adds `entity:<add_method>()` to the already-bound Entity usertype.
    //   2. Records that method in the meta registry for IDE autocomplete.
    //   3. Records the schema entry (prefab `key` + setters) for the prefab applier.
    // bind_entity() must have run first so the Entity usertype exists.
    template<typename T>
    void bind_component_schema(sol::state& lua,
                               LuaMetaRegistry& meta,
                               LuaComponentSchemaRegistry& schemas,
                               const EntitySpawner& spawner,
                               const char* key,
                               const char* add_method,
                               std::initializer_list<std::pair<const char*, const char*>> setters) {
        const char* entity_lua_name = LuaTypeName<EntityHandle>::value;
        sol::table entity_ut = lua[entity_lua_name];
        entity_ut[add_method] = [&spawner](const EntityHandle& h) -> T* {
            Entity* e = spawner.get_entity(h.id);
            return e ? e->add_component<T>() : nullptr;
        };

        if (LuaUsertypeInfo* entity_info = meta.find_usertype(entity_lua_name)) {
            LuaMethodInfo info;
            info.name = add_method;
            info.ret = meta_detail::lua_name<T*>();
            info.is_static = false;
            entity_info->methods.push_back(std::move(info));
        }

        std::vector<std::pair<std::string, std::string>> v;
        v.reserve(setters.size());
        for (const auto& s : setters) {
            v.emplace_back(s.first, s.second);
        }

        schemas.add_schema(key, add_method, std::move(v));
    }
}
