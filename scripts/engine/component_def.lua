-- DefineComponent: Class declaration for Lua components.
--
-- Usage:
--   DefineComponent.Player = {
--       speed = 7.0,                          -- scalar default; per-instance via Lua shadowing
--       __parents = { "Base" },               -- optional inheritance
--   }
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
    local class = { __component_name = name }

    -- Inherit from parents first; child overrides.
    if def.__parents then
        for _, parent_name in ipairs(def.__parents) do
            local parent = _G.__component_registry[parent_name]
            if not parent then
                log_error("DefineComponent." .. name .. ": parent '" .. parent_name .. "' is not registered")
            else
                for k, v in pairs(parent) do
                    if k ~= "__index" and k ~= "new" and k ~= "__component_name" then
                        class[k] = v
                    end
                end
            end
        end
    end

    -- Copy def fields onto the class. Methods and shared defaults all live here.
    for k, v in pairs(def) do
        if k ~= "__parents" then
            class[k] = v
        end
    end

    class.__index = class

    -- Init per-instance data.
    function class.new()
        local self = setmetatable({}, class)
        if class.init then
            class.init(self)
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
