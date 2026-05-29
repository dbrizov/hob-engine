-- Lua-side bootstrap. The C++ side runs ONLY this file; all script loading
-- (engine modules, user scripts, main.lua) is orchestrated from here.

-- Start the Lua debugger when launched under the VS Code Lua debugger extension.
if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end

-- Engine modules just install registries / metatables / enums.
Scripts.run_folder("scripts/engine", { "bootstrap.lua" })

-- User scripts: anything outside engine/, lib/, meta/, and main.lua.
-- Mixins, components, prefabs, and behavior scripts can live anywhere under scripts/.
Scripts.run_folder("scripts", { "engine", "lib", "meta", "main.lua" })

-- Resolve __parent / __mixins for every DefineComponent now that all files
-- (and thus all DefineMixin / DefineComponent registrations) are loaded.
build_component_classes()

-- Entry point.
Scripts.run_file("scripts/main.lua")
