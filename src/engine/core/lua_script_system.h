#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <sol/sol.hpp>

#include "lua_meta.h"

namespace hob {
    class Engine;

    class LuaScriptSystem {
        Engine& m_engine;
        sol::state m_lua;
        LuaMetaRegistry m_meta;

    public:
        explicit LuaScriptSystem(Engine& engine);

        LuaScriptSystem(const LuaScriptSystem&) = delete;
        LuaScriptSystem& operator=(const LuaScriptSystem&) = delete;

        sol::state& get_lua();

    private:
        bool run_file(const std::filesystem::path& relative_path);
        bool run_folder(const std::filesystem::path& relative_path, const std::vector<std::string>& excludes = {});
        bool run_bootstrap();

        void register_bindings();
        void dump_meta();

        void bind_math();
        void bind_entity();
        void bind_components();
        void bind_subsystems();
        void bind_debug();
    };
}
