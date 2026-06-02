-- DefineMaterial: Named alias for a Material definition.
--
-- Usage:
--   DefineMaterial.Outline = {
--       shader = Assets.OutlineShader,
--       tint = Color(1, 0, 0, 1),
--   }
--
-- Then in a prefab:
--   sprite = {
--       texture = Assets.RobotTexture,
--       material = Materials.Outline,
--   }
--
-- `Materials.Name` returns a deferred reference. The actual C++ Material is
-- constructed lazily on first resolve (and cached). DefineMaterial calls can live
-- in any file in any load order, same contract as DefineAsset / DefineAnimationClip.

_G.__material_defs = _G.__material_defs or {}
_G.__materials_built = _G.__materials_built or {}

local function build_material(name)
    local def = _G.__material_defs[name]
    if not def then
        Debug.log_error("Material '" .. name .. "' is not defined")
        return nil
    end

    local mat = Material {
        shader = def.shader and Assets.resolve(def.shader) or nil,
        tint = def.tint,
    }

    _G.__materials_built[name] = mat
    return mat
end

local material_ref_mt = {
    __tostring = function(self)
        return "Material(" .. tostring(self.__material) .. ")"
    end,
}

_G.DefineMaterial = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            Debug.log_error("DefineMaterial." .. tostring(name) .. " must be assigned a table")
            return
        end

        _G.__material_defs[name] = def
    end,
})

_G.Materials = setmetatable({}, {
    __index = function(t, name)
        local wrapper = setmetatable({ __material = name }, material_ref_mt)
        rawset(t, name, wrapper)
        return wrapper
    end,
})

rawset(_G.Materials, "resolve", function(value)
    if type(value) == "table" and getmetatable(value) == material_ref_mt then
        local name = value.__material
        local built = _G.__materials_built[name]
        if built then
            return built
        end
        return build_material(name)
    end

    return value
end)
