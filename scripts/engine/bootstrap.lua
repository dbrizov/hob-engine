-- Lua-side bootstrap. The C++ side runs ONLY this file; all script loading
-- (engine modules, user scripts, main.lua) is orchestrated from here.

-- Start the Lua debugger when launched under the VS Code Lua debugger extension.
if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end

-- Engine modules just install registries / metatables / enums.
-- lib/ is `require`d on demand (vendored lldebugger); meta/ is LuaCATS annotations.
Scripts.run_folder("scripts/engine", { "bootstrap.lua", "lib", "meta" })

-- Install DefineAsset / DefineTexture / DefineShader / ... from the generated path schema.
-- Then DefineMaterial / DefineAnimationClip / ... from the generated factory schema.
-- Runs after the engine folder (so *_schemas.generated.lua and *_def.lua are loaded)
-- and before user scripts (so they may call DefineTexture.X etc. at file scope).
-- Path registries first: factory configs (e.g. Materials) may reference Textures/Shaders.
install_path_registries()
install_factory_registries()

-- User scripts: anything outside engine/ and main.lua.
-- Assets, materials, animation clips, entities, components, and mixins can live anywhere under scripts/.
Scripts.run_folder("scripts", { "engine", "main.lua" })

-- Resolve __parent / __mixins for every DefineComponent now that all files
-- (and thus all DefineComponent / DefineMixin registrations) are loaded.
finalize_components()

-- Entry point.
Scripts.run_file("scripts/main.lua")
