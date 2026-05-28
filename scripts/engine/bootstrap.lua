-- Lua-side bootstrap. The C++ side runs ONLY this file; all script loading
-- (engine modules, user scripts, main.lua) is orchestrated from here.

local engine_modules = {
    "scripts/engine/collision.lua",
    "scripts/engine/mixin_def.lua", -- must come before component_def: build_class consults __mixin_registry
    "scripts/engine/component_def.lua",
    "scripts/engine/component_schemas.lua",
    "scripts/engine/entity_def.lua",
}

for _, path in ipairs(engine_modules) do
    Scripts.run_file(path)
end

-- User scripts: everything under scripts/, excluding engine internals,
-- vendored libs, IDE-only meta, and the entry point itself.
Scripts.run_folder("scripts", { "engine", "lib", "meta", "main.lua" })

-- Entry point.
Scripts.run_file("scripts/main.lua")
