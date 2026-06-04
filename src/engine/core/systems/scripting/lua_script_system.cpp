#include "lua_script_system.h"
#include "lua_script_system_impl.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
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

        // Schema files are consumed by the Lua bootstrap, so they must be written first.
        dump_component_schemas();
        dump_path_schemas();
        dump_factory_schemas();

        run_bootstrap();

#ifndef NDEBUG
        // Meta files are LuaCATS-only (no runtime effect), so they're written after bootstrap.
        // dump_path_aliases_meta in particular needs the user-script `DefineTexture.Foo = "..."`
        // lines to have already run.
        dump_meta();
        dump_path_schemas_meta();
        dump_path_aliases_meta();
        dump_factory_schemas_meta();
        dump_factory_aliases_meta();
#endif
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
        bind_assets();
        bind_debug();
    }

    void LuaScriptSystem::dump_component_schemas() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "component_schemas.generated.lua";

        if (!m_impl->component_schemas.write_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_component_schemas: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_path_schemas() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "path_schemas.generated.lua";

        if (!m_impl->path_schemas.write_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_path_schemas: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_factory_schemas() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "factory_schemas.generated.lua";

        if (!m_impl->factory_schemas.write_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_factory_schemas: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_meta() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "meta" / "engine_meta.generated.lua";

        if (!m_impl->meta.write_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_meta: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_path_schemas_meta() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "meta" / "path_schemas_meta.generated.lua";

        if (!m_impl->path_schemas.write_meta_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_path_schemas_meta: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_path_aliases_meta() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "meta" / "path_aliases_meta.generated.lua";

        sol::object names_obj = m_impl->lua["__path_alias_names"];
        sol::table names_by_registry = names_obj.is<sol::table>() ? names_obj.as<sol::table>() : sol::table{};

        std::ostringstream out;
        out << "---@meta\n";
        out << "-- AUTO-GENERATED by LuaScriptSystem::dump_path_aliases_meta. Do not edit by hand.\n";
        out << "-- Source of truth: `DefineX.Foo = \"...\"` assignments throughout scripts/.\n";
        out << "-- Adds `---@field` entries to each registry class (Textures, Shaders, Assets, ...)\n";
        out << "-- so editors autocomplete `Textures.Foo`, etc. LuaCATS merges these into the\n";
        out << "-- registry classes declared in path_schemas_meta.generated.lua.\n\n";

        for (const auto& s : m_impl->path_schemas.get_schemas()) {
            std::vector<std::string> names;
            if (names_by_registry.valid()) {
                sol::object entry = names_by_registry[s.registry_name];
                if (entry.is<sol::table>()) {
                    sol::table t = entry.as<sol::table>();
                    for (std::size_t i = 1; i <= t.size(); ++i) {
                        sol::object v = t[i];
                        if (v.is<std::string>()) {
                            names.push_back(v.as<std::string>());
                        }
                    }
                }
            }
            std::sort(names.begin(), names.end());

            out << "---@class " << s.registry_name << "\n";
            for (const auto& n : names) {
                out << "---@field " << n << " string\n";
            }
            out << "\n";
        }

        std::error_code ec;
        std::filesystem::create_directories(out_path.parent_path(), ec);

        std::ofstream f(out_path, std::ios::binary | std::ios::trunc);
        if (!f) {
            debug::log_error("LuaScriptSystem::dump_path_aliases_meta: failed to open '{}' for writing",
                             out_path.string());
            return;
        }

        const std::string str = out.str();
        f.write(str.data(), static_cast<std::streamsize>(str.size()));

        if (!f.good()) {
            debug::log_error("LuaScriptSystem::dump_path_aliases_meta: write failed for '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_factory_schemas_meta() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "meta" / "factory_schemas_meta.generated.lua";

        if (!m_impl->factory_schemas.write_meta_to_file(out_path)) {
            debug::log_error("LuaScriptSystem::dump_factory_schemas_meta: failed to write '{}'", out_path.string());
        }
    }

    void LuaScriptSystem::dump_factory_aliases_meta() {
        const std::filesystem::path out_path =
            PathUtils::get_root_path() / "scripts" / "engine" / "meta" / "factory_aliases_meta.generated.lua";

        sol::object names_obj = m_impl->lua["__factory_alias_names"];
        sol::table names_by_registry = names_obj.is<sol::table>() ? names_obj.as<sol::table>() : sol::table{};

        std::ostringstream out;
        out << "---@meta\n";
        out << "-- AUTO-GENERATED by LuaScriptSystem::dump_factory_aliases_meta. Do not edit by hand.\n";
        out << "-- Source of truth: `DefineX.Foo = { ... }` assignments throughout scripts/.\n";
        out << "-- Declares the `X` registry classes (e.g. Materials, AnimationClips) and adds\n";
        out << "-- `---@field` entries so editors autocomplete `Materials.Foo`, etc.\n\n";

        for (const auto& s : m_impl->factory_schemas.get_schemas()) {
            std::vector<std::string> names;
            if (names_by_registry.valid()) {
                sol::object entry = names_by_registry[s.registry_name];
                if (entry.is<sol::table>()) {
                    sol::table t = entry.as<sol::table>();
                    for (std::size_t i = 1; i <= t.size(); ++i) {
                        sol::object v = t[i];
                        if (v.is<std::string>()) {
                            names.push_back(v.as<std::string>());
                        }
                    }
                }
            }
            std::sort(names.begin(), names.end());

            out << "---@class " << s.registry_name << "\n";
            for (const auto& n : names) {
                out << "---@field " << n << " " << s.lua_type << "\n";
            }
            out << "\n";
        }

        std::error_code ec;
        std::filesystem::create_directories(out_path.parent_path(), ec);

        std::ofstream f(out_path, std::ios::binary | std::ios::trunc);
        if (!f) {
            debug::log_error("LuaScriptSystem::dump_factory_aliases_meta: failed to open '{}' for writing",
                             out_path.string());
            return;
        }

        const std::string str = out.str();
        f.write(str.data(), static_cast<std::streamsize>(str.size()));

        if (!f.good()) {
            debug::log_error("LuaScriptSystem::dump_factory_aliases_meta: write failed for '{}'", out_path.string());
        }
    }
}
