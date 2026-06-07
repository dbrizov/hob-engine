-- Lua hot reload. Running this file IS the reload: it rebuilds every component /
-- mixin class from the current source files and re-points all live instances at
-- the rebuilt classes, preserving per-instance state.
--
-- This file is imperative top-level code, so it is excluded from the normal
-- bootstrap load (see bootstrap.lua) and is only executed when C++ explicitly
-- runs it via LuaScriptSystem::reload().
--
-- State survives for free: per-instance state lives as raw fields on the instance
-- table, while methods/defaults live on the class table reached via the metatable.
-- We only swap the metatable; we do NOT re-run init().

-- 1. Wipe registries so definitions rebuild cleanly.
_G.__component_registry = {}
_G.__component_pending = {}
_G.__mixin_registry = {}

-- 2. Re-run the same definition files bootstrap uses, then re-finalize.
--    (Lua owns the file set via load_game_definitions — C++ never sees it.)
load_game_definitions()

-- 3. Re-point every live instance at its rebuilt class. Optional on_hot_reload()
--    lets a component migrate state when a field's shape changed.
--    Weak-keyed set, so iterate keys: `for inst in pairs(...)`.
for inst in pairs(_G.__live_instances) do
    local class = _G.__component_registry[inst.class_name]
    if class then
        setmetatable(inst, class)
        if type(inst.on_hot_reload) == "function" then
            inst:on_hot_reload()
        end
    end
end

Debug.print("Lua hot reload complete", Color.white(), 4.0)
