-- DefineAsset: Named alias for asset paths.
--
-- Usage:
--   DefineAsset.PlayerTexture = "images/player/HJ_run01.png"
--   DefineAsset.RobotTexture  = "images/robot.png"
--
-- Then in prefabs / config:
--   sprite = { texture = Assets.PlayerTexture }
--   Cursor.config { texture = Assets.CursorTexture }
--
-- Paths are relative to the assets/ root, same as a raw string would be.
--
-- `Assets.Name` returns a deferred reference, not an eager string. The actual
-- path lookup happens at dispatch time (apply_setters in entity_def.lua and
-- Cursor.config in cursor.lua), so DefineAsset calls can live in any file in
-- any load order. If user-script code calls a C++ setter directly with an
-- Assets.X value, it must pass it through resolve_asset(...) or tostring(...).

_G.__assets = _G.__assets or {}

local asset_ref_mt = {
    __tostring = function(self)
        local path = _G.__assets[self.__asset]
        if not path then
            Debug.log_error("Asset '" .. self.__asset .. "' is not defined")
            return ""
        end

        return path
    end,
    __resolve = function(self)
        local path = _G.__assets[self.__asset]
        if not path then
            Debug.log_error("Asset '" .. self.__asset .. "' is not defined")
            return ""
        end

        return path
    end,
}

_G.DefineAsset = setmetatable({}, {
    __newindex = function(_, name, def)
        if def == nil then
            Debug.log_error("DefineAsset." .. tostring(name) .. " must not be nil")
            return
        end

        _G.__assets[name] = def
    end,
})

_G.Assets = setmetatable({}, {
    __index = function(t, name)
        local wrapper = setmetatable({ __asset = name }, asset_ref_mt)
        rawset(t, name, wrapper)
        return wrapper
    end,
})
