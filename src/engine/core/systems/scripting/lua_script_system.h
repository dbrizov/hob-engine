#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace sol {
    class state;
}

namespace hob {
    class Engine;
    struct LuaScriptSystemImpl;

    class LuaScriptSystem {
        Engine& m_engine;
        std::unique_ptr<LuaScriptSystemImpl> m_impl;

    public:
        explicit LuaScriptSystem(Engine& engine);
        ~LuaScriptSystem();

        LuaScriptSystem(const LuaScriptSystem&) = delete;
        LuaScriptSystem& operator=(const LuaScriptSystem&) = delete;

        sol::state& get_lua();

    private:
        bool run_file(const std::filesystem::path& relative_path);
        bool run_folder(const std::filesystem::path& relative_path, const std::vector<std::string>& excludes = {});
        bool run_bootstrap();

        void register_bindings();

        void bind_math();
        void bind_entity();
        void bind_components();
        void bind_systems();
        void bind_assets();
        void bind_debug();

        void dump_component_schemas();
        void dump_path_schemas();
        void dump_factory_schemas();

        void dump_meta();
        void dump_path_schemas_meta();
        void dump_path_aliases_meta();
        void dump_factory_schemas_meta();
        void dump_factory_aliases_meta();
    };
}
