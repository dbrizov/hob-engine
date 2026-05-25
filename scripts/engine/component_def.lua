-- DefineComponent: Surviving Mars-style class declaration for Lua components.
--
-- Usage:
--   DefineComponent.Player = {
--       speed = 7.0,                                             -- scalar default
--       movement_input = function() return Vector2.zero() end,   -- per-instance factory
--       __parents = { "Base" },                                  -- optional inheritance
--   }
--
--   function Player:enter_play() ... end
--   function Player:physics_tick(dt) ... end
--
-- Lifecycle method names (enter_play, exit_play, tick, physics_tick,
-- debug_draw_tick, on_collision_enter/exit, on_trigger_enter/exit) declared
-- as functions in the def table become methods. Any OTHER function in the
-- def table is treated as a per-instance default factory and is called when
-- new() is invoked. Tables are shared across instances (use a factory if you
-- need per-instance state).

_G.__component_registry = _G.__component_registry or {}

local lifecycle_hooks = {
    enter_play = true,
    exit_play = true,
    tick = true,
    physics_tick = true,
    debug_draw_tick = true,
    on_collision_enter = true,
    on_collision_exit = true,
    on_trigger_enter = true,
    on_trigger_exit = true,
}

local function classify(def)
    -- Returns (methods, defaults, factories) tables.
    local methods = {}
    local defaults = {}
    local factories = {}
    for k, v in pairs(def) do
        if k == "__parents" then
            -- handled separately
        elseif type(v) == "function" then
            if lifecycle_hooks[k] then
                methods[k] = v
            else
                factories[k] = v
            end
        else
            defaults[k] = v
        end
    end

    return methods, defaults, factories
end

local function build_class(name, def)
    local class = {
        __component_name = name,
        __methods = {},
        __defaults = {},
        __factories = {},
    }

    -- Inherit from parents first (child overrides).
    local parents = def.__parents
    if parents then
        for _, parent_name in ipairs(parents) do
            local parent = _G.__component_registry[parent_name]
            if not parent then
                log_error("DefineComponent." .. name .. ": parent '" .. parent_name .. "' is not registered")
            else
                for k, v in pairs(parent.__methods) do
                    class[k] = v
                end
                for k, v in pairs(parent.__defaults) do
                    class.__defaults[k] = v
                end
                for k, v in pairs(parent.__factories) do
                    class.__factories[k] = v
                end
            end
        end
    end

    local methods, defaults, factories = classify(def)

    for k, v in pairs(methods) do
        class[k] = v
        class.__methods[k] = v
    end
    for k, v in pairs(defaults) do
        class.__defaults[k] = v
    end
    for k, v in pairs(factories) do
        class.__factories[k] = v
    end

    class.__index = class

    function class.new()
        local self = setmetatable({}, class)
        for k, v in pairs(class.__defaults) do
            self[k] = v
        end
        for k, factory in pairs(class.__factories) do
            self[k] = factory()
        end
        return self
    end

    return class
end

_G.DefineComponent = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            log_error("DefineComponent." .. tostring(name) .. " must be assigned a table")
            return
        end

        local class = build_class(name, def)
        _G.__component_registry[name] = class
        _G[name] = class
    end,
    __index = function(_, name)
        return _G.__component_registry[name]
    end,
})
