#include "lua_script_system.h"
#include "lua_script_system_impl.h"

#include <algorithm>
#include <vector>

#include "engine/core/logging.h"
#include "engine/core/path_utils.h"

namespace hob {
    LuaScriptSystem::LuaScriptSystem(Engine& engine)
        : m_engine(engine)
        , m_impl(std::make_unique<LuaScriptSystemImpl>()) {
        sol::state& lua = m_impl->lua;
        lua.open_libraries(
            sol::lib::base,
            sol::lib::string,
            sol::lib::math,
            sol::lib::table,
            sol::lib::io,
            sol::lib::os,
            sol::lib::package,
            sol::lib::coroutine,
            sol::lib::debug);

        // Make `require` find modules in scripts/engine/lib (e.g. vendored lldebugger).
        const std::string lib_path = (PathUtils::get_root_path() / "scripts" / "engine" / "lib" / "?.lua").string();
        sol::table package = lua["package"];
        package["path"] = lib_path + ";" + package["path"].get<std::string>();

        register_bindings();
#ifndef NDEBUG
        dump_meta();
#endif
        dump_component_schemas();
        run_bootstrap();
    }

    LuaScriptSystem::~LuaScriptSystem() = default;

    sol::state& LuaScriptSystem::get_lua() {
        return m_impl->lua;
    }

    bool LuaScriptSystem::run_file(const std::filesystem::path& relative_path) {
        const std::filesystem::path full_path = PathUtils::get_root_path() / relative_path;

        auto result = m_impl->lua.safe_script_file(full_path.string(), sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            debug::log_error("Lua error in {}: {}", full_path.string(), err.what());
            return false;
        }

        return true;
    }

    bool LuaScriptSystem::run_folder(const std::filesystem::path& relative_path,
                                     const std::vector<std::string>& excludes) {
        const std::filesystem::path root = PathUtils::get_root_path() / relative_path;
        if (!std::filesystem::exists(root)) {
            debug::log_error("LuaScriptSystem::run_folder: '{}' does not exist", root.string());
            return false;
        }

        auto is_excluded = [&](const std::filesystem::path& p) {
            const std::filesystem::path rel = std::filesystem::relative(p, root);
            for (const auto& part : rel) {
                const std::string s = part.string();
                for (const auto& name : excludes) {
                    if (s == name) {
                        return true;
                    }
                }
            }

            return false;
        };

        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".lua") {
                continue;
            }

            if (is_excluded(entry.path())) {
                continue;
            }

            files.push_back(entry.path());
        }

        std::sort(files.begin(), files.end());

        bool all_ok = true;
        for (const auto& file : files) {
            auto result = m_impl->lua.safe_script_file(file.string(), sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                debug::log_error("Lua error in {}: {}", file.string(), err.what());
                all_ok = false;
            }
        }

        return all_ok;
    }

    bool LuaScriptSystem::run_bootstrap() {
        return run_file("scripts/engine/bootstrap.lua");
    }

    void LuaScriptSystem::register_bindings() {
        bind_math();
        bind_entity();
        bind_components();
        bind_systems();
        bind_debug();
    }

    void LuaScriptSystem::dump_meta() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "meta" / "engine.generated.lua";

        if (!m_impl->meta.write_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_meta: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_component_schemas() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "component_schemas.generated.lua";

        if (!m_impl->component_schemas.write_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_component_schemas: failed to write '{}'", out_path.string());
        }
    }
}
