#include "lua_script_system.h"

#include "logging.h"
#include "path_utils.h"
#include "engine/scripting/lua_bindings.h"

namespace hob {
    LuaScriptSystem::LuaScriptSystem(App& app)
        : m_app(app) {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::math,
            sol::lib::table,
            sol::lib::io,
            sol::lib::os);

        // Add scripts/ (next to the executable in release, project root in debug)
        // and scripts/?/init.lua to package.path so require() can find modules.
        const std::filesystem::path scripts_root = PathUtils::get_root_path() / "scripts";
        const std::string scripts_root_str = scripts_root.generic_string();
        const std::string current_path = m_lua["package"]["path"];
        m_lua["package"]["path"] =
            current_path + ";" +
            scripts_root_str + "/?.lua;" +
            scripts_root_str + "/?/init.lua";

        register_bindings(m_lua, app);
    }

    bool LuaScriptSystem::run_file(const std::filesystem::path& path) {
        const std::filesystem::path full_path = path.is_absolute() ? path : PathUtils::get_root_path() / path;

        auto result = m_lua.safe_script_file(full_path.string(), sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            debug::log_error("Lua error in {}: {}", full_path.string(), err.what());
            return false;
        }

        return true;
    }

    sol::state& LuaScriptSystem::get_lua() {
        return m_lua;
    }
}
