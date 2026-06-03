-- Generic factory-type registry.
--
-- Drives DefineMaterial / DefineAnimationClip (and any future DefineX where X is a
-- C++ usertype bound with a `factory_ctor` that takes a single config table).
-- The per-type field list lives in factory_schemas.generated.lua, which is dumped
-- from C++ by LuaScriptSystem::dump_factory_schemas() — to add a new factory type,
-- call bind_factory_schema(...) next to its bind_usertype<T>() and the Lua side
-- picks it up on the next run; no edits to this file are needed.
--
-- Usage (same contract as DefineAsset):
--   DefineMaterial.Outline = { shader = Assets.OutlineShader, tint = Color(1,0,0,1) }
--   sprite = { material = Materials.Outline }
--
-- `Materials.Name` / `AnimationClips.Name` return deferred refs. The actual C++
-- object is constructed lazily on first resolve and cached. Define calls can live
-- in any file in any load order.

local resolvers = {
    passthrough = function(v) return v end,
    asset = function(v)
        if v == nil then return nil end
        return Assets.resolve(v)
    end,
    asset_list = function(v)
        if v == nil then return nil end
        local out = {}
        for i, entry in ipairs(v) do
            out[i] = Assets.resolve(entry)
        end
        return out
    end,
}

local function install_factory(registry_name, schema)
    local defs = {}
    local built = {}

    local ref_mt = {
        __tostring = function(self)
            return schema.lua_type .. "(" .. self.__name .. ")"
        end,
    }

    local function build(name)
        local def = defs[name]
        if not def then
            Debug.log_error(schema.lua_type .. " '" .. name .. "' is not defined")
            return nil
        end

        local cfg = {}
        for _, field in ipairs(schema.fields) do
            local resolver = resolvers[field.resolve]
            cfg[field.name] = resolver(def[field.name])
        end

        local ctor = _G[schema.lua_type]
        if ctor == nil then
            Debug.log_error("Factory type '" .. schema.lua_type .. "' is not bound in Lua")
            return nil
        end

        local obj = ctor(cfg)
        built[name] = obj
        return obj
    end

    _G[schema.define] = setmetatable({}, {
        __newindex = function(_, name, def)
            if type(def) ~= "table" then
                Debug.log_error(schema.define .. "." .. tostring(name) .. " must be assigned a table")
                return
            end

            defs[name] = def
        end,
    })

    local registry = setmetatable({}, {
        __index = function(t, name)
            local wrapper = setmetatable({ __name = name }, ref_mt)
            rawset(t, name, wrapper)
            return wrapper
        end,
    })

    rawset(registry, "resolve", function(value)
        if type(value) == "table" and getmetatable(value) == ref_mt then
            return built[value.__name] or build(value.__name)
        end

        return value
    end)

    _G[registry_name] = registry
end

function _G.install_factories()
    local schemas = _G.__factory_schemas
    if schemas == nil then
        Debug.log_error("install_factories: __factory_schemas is missing (did factory_schemas.generated.lua run?)")
        return
    end

    for registry_name, schema in pairs(schemas) do
        install_factory(registry_name, schema)
    end
end
