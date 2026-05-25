#pragma once

#include <filesystem>
#include <initializer_list>
#include <string_view>

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

        sol::state& get_lua();

    private:
        bool run_file(const std::filesystem::path& path);
        bool run_folder(const std::filesystem::path& path, std::initializer_list<std::string_view> excludes = {});
        bool run_bootstrap();

        void register_bindings();

        void bind_math();
        void bind_entity();
        void bind_components();
        void bind_subsystems();
        void bind_logging();
    };
}
