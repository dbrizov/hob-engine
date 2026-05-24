#pragma once

#include <filesystem>

#include <sol/sol.hpp>

namespace hob {
    class App;

    class LuaScriptSystem {
        App& m_app;
        sol::state m_lua;

    public:
        explicit LuaScriptSystem(App& app);

        LuaScriptSystem(const LuaScriptSystem&) = delete;
        LuaScriptSystem& operator=(const LuaScriptSystem&) = delete;

        bool run_file(const std::filesystem::path& path);

        sol::state& get_lua();
    };
}
