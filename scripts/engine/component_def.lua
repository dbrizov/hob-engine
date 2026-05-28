-- DefineComponent: Class declaration for Lua components.
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

local function build_class(name, def)
    local class = {}

    -- 1. Single parent (must be a registered Component). Subclass may freely override.
    if def.__parent then
        if type(def.__parent) ~= "string" then
            log_error("DefineComponent." .. name .. ": __parent must be a string component name")
            return class
        end
        local parent = _G.__component_registry[def.__parent]
        if not parent then
            log_error("DefineComponent." .. name .. ": parent '" .. def.__parent .. "' is not registered")
        else
            for k, v in pairs(parent) do
                if k ~= "__index" and k ~= "new" then
                    class[k] = v
                end
            end
        end
    end

    -- 2. Mixins: orthogonal capabilities. Any key collision with the parent or
    --    a previously-applied mixin is an error. The def itself may override.
    if def.__mixins then
        for _, mixin_name in ipairs(def.__mixins) do
            if type(mixin_name) ~= "string" then
                log_error("DefineComponent." .. name .. ": __mixins entries must be string mixin names")
                return class
            end
            local mixin = _G.__mixin_registry and _G.__mixin_registry[mixin_name]
            if not mixin then
                log_error("DefineComponent." .. name .. ": mixin '" .. mixin_name .. "' is not registered")
            else
                for k, v in pairs(mixin) do
                    if class[k] ~= nil then
                        log_error("DefineComponent." .. name ..
                            ": mixin '" .. mixin_name .. "' key '" .. k ..
                            "' collides with an existing definition (from parent or earlier mixin)")
                    else
                        class[k] = v
                    end
                end
            end
        end
    end

    -- 3. Copy def fields onto the class. Methods and shared defaults all live here.
    --    These intentionally override anything inherited from parent or mixins.
    for k, v in pairs(def) do
        if k ~= "__parent" and k ~= "__mixins" then
            class[k] = v
        end
    end

    class.__index = class

    -- Allocates an instance. The engine sets self.entity and calls init()
    -- from C++ after this returns, so do not invoke init() here.
    function class.new()
        return setmetatable({}, class)
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
