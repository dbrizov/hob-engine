-- Lua-side bootstrap. The C++ side runs ONLY this file; all script loading
-- (engine modules, user scripts, main.lua) is orchestrated from here.

-- Start the Lua debugger when launched under the VS Code Lua debugger extension.
if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end

-- Engine modules just install registries / metatables / enums.
-- lib/ is `require`d on demand (vendored lldebugger); meta/ is LuaCATS annotations.
Scripts.run_folder("scripts/engine", { "bootstrap.lua", "lib", "meta" })

-- User scripts: anything outside engine/ and main.lua.
-- Mixins, components, prefabs, and behavior scripts can live anywhere under scripts/.
Scripts.run_folder("scripts", { "engine", "main.lua" })

-- Resolve __parent / __mixins for every DefineComponent now that all files
-- (and thus all DefineMixin / DefineComponent registrations) are loaded.
build_component_classes()

-- Entry point.
Scripts.run_file("scripts/main.lua")
