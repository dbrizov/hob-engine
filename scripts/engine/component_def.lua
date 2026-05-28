-- DefineComponent: Class declaration for Lua components.
--
-- Components are built in two phases:
--   1. DefineComponent.X = {...}  -- registers a placeholder class table immediately
--                                    so `local X = X` and `function X:foo() end` work.
--   2. build_component_classes()  -- called by bootstrap.lua after ALL user scripts have
--                                    loaded. Resolves __parent and __mixins for every
--                                    pending component, walking the inheritance graph.
--
-- This means DefineMixin / DefineComponent calls can appear in ANY file in ANY order;
-- references only need to be resolvable by the time build_component_classes() runs.
--
-- Usage:
--   DefineComponent.Player = {
--       speed = 7.0,                          -- scalar default; per-instance via Lua shadowing
--       priority = 1,                         -- priority execution order for this type of component; NOT per-instance data
--       __parent = "Character",               -- optional single-inheritance (must be a registered Component)
--       __mixins = { "Damageable" },          -- optional orthogonal capabilities (see mixin_def.lua)
--   }
--
--   ---@class Player : Character              -- these 2 lines are a hint to the LuaLS
--   local Player = Player                     -- so that we can see intellisense from the inherited component
--
--   function Player:init() ... end            -- per-instance setup (mutable state goes here)
--       self.speed = 10.0                     -- override the default speed
--   }
--
--   function Player:enter_play() ... end
--   function Player:exit_play() ... end
--   function Player:tick(delta_time) ... end
--   function Player:my_helper() ... end       -- any user method

_G.__component_registry = _G.__component_registry or {}
_G.__component_pending = _G.__component_pending or {}

local function build_class(name)
    if _G.__component_registry[name] then
        return _G.__component_registry[name]
    end

    local pending = _G.__component_pending[name]
    if not pending then
        return nil
    end

    if pending.building then
        log_error("DefineComponent." .. name .. ": cyclic inheritance detected")
        return pending.class
    end
    pending.building = true

    local class = pending.class
    local def = pending.def

    -- 1. Single parent (must be a registered Component). Resolve recursively so
    --    parents in any file/load-order get built before children. Subclass-on-class
    --    methods (defined via `function X:foo()`) are already on `class`; parent
    --    entries that collide are skipped (override semantics).
    local parent_keys = {}
    if def.__parent then
        if type(def.__parent) ~= "string" then
            log_error("DefineComponent." .. name .. ": __parent must be a string component name")
        else
            build_class(def.__parent)
            local parent = _G.__component_registry[def.__parent]
            if not parent then
                log_error("DefineComponent." .. name .. ": parent '" .. def.__parent .. "' is not registered")
            else
                for k, v in pairs(parent) do
                    if k ~= "__index" and k ~= "new" then
                        if rawget(class, k) == nil then
                            class[k] = v
                        end

                        parent_keys[k] = true
                    end
                end
            end
        end
    end

    -- 2. Mixins: orthogonal capabilities. Collisions between mixins, or between a
    --    mixin and the parent, are errors. The host component may explicitly
    --    override a mixin method (defining `function X:foo()` after DefineComponent)
    --    and that's allowed silently.
    if def.__mixins then
        local mixin_keys = {}
        for _, mixin_name in ipairs(def.__mixins) do
            if type(mixin_name) ~= "string" then
                log_error("DefineComponent." .. name .. ": __mixins entries must be string mixin names")
            else
                local mixin = _G.__mixin_registry and _G.__mixin_registry[mixin_name]
                if not mixin then
                    log_error("DefineComponent." .. name .. ": mixin '" .. mixin_name .. "' is not registered")
                else
                    for k, v in pairs(mixin) do
                        if parent_keys[k] or mixin_keys[k] then
                            log_error("DefineComponent." .. name ..
                                ": mixin '" .. mixin_name .. "' key '" .. k ..
                                "' collides with an existing definition (from parent or earlier mixin)")
                        elseif rawget(class, k) == nil then
                            class[k] = v
                        end

                        mixin_keys[k] = true
                    end
                end
            end
        end
    end

    class.__index = class

    -- Allocates an instance. The engine sets self.entity and calls init()
    -- from C++ after this returns, so do not invoke init() here.
    function class.new()
        return setmetatable({}, class)
    end

    _G.__component_registry[name] = class
    pending.building = false

    return class
end

function _G.build_component_classes()
    local names = {}
    local n = 0
    for name in pairs(_G.__component_pending) do
        n = n + 1
        names[n] = name
    end

    for _, name in ipairs(names) do
        build_class(name)
    end

    _G.__component_pending = {}
end

_G.DefineComponent = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            log_error("DefineComponent." .. tostring(name) .. " must be assigned a table")
            return
        end

        -- Create the class placeholder immediately so that `local X = X` and
        -- subsequent `function X:foo()` calls in the same file work as expected.
        -- Copy non-meta fields from def onto the placeholder so scalar defaults
        -- (e.g. `speed = 7.0`) are visible right away.
        local class = {}
        for k, v in pairs(def) do
            if k ~= "__parent" and k ~= "__mixins" then
                class[k] = v
            end
        end

        _G.__component_pending[name] = { class = class, def = def }
        _G[name] = class
    end,
    __index = function(_, name)
        local class = _G.__component_registry[name]
        if class then return class end

        local pending = _G.__component_pending[name]
        return pending and pending.class or nil
    end,
})
