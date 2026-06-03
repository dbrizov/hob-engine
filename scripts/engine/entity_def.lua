-- DefineEntity: Prefab declaration for entities.
--
-- Usage:
--   DefineEntity.Player = {
--       ticking = true,
--       character_body = {
--           collision_layer = Collision.Kinematic,
--           collision_mask  = Collision.Static | Collision.Dynamic | Collision.Trigger,
--           capsule         = Capsule(Vector2.zero(), Vector2.zero(), 1.2),
--       },
--       sprite = { texture = Assets.PlayerTexture, z_index = 1 },
--       input = {},
--       lua_components = { "Player" },
--   }

_G.__entity_prefab_registry = _G.__entity_prefab_registry or {}

_G.DefineEntity = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            Debug.log_error("DefineEntity." .. tostring(name) .. " must be assigned a table")
            return
        end

        _G.__entity_prefab_registry[name] = def
    end,
    __index = function(_, name)
        return _G.__entity_prefab_registry[name]
    end,
})

local function apply_setters(component, section, setters)
    for prop, value in pairs(section) do
        local resolved_value = resolve_asset(value)
        local setter = setters[prop]
        if setter == nil then
            Debug.log_error("Unknown prefab property '" .. tostring(prop) .. "' for component")
        elseif type(setter) == "string" then
            component[setter](component, resolved_value)
        else
            setter(component, resolved_value)
        end
    end
end

local function apply_prefab(entity, prefab)
    local schemas = _G.__component_schemas

    if prefab.ticking ~= nil then
        entity:set_ticking(prefab.ticking)
    end

    for _, key in ipairs(schemas.__order) do
        local section = prefab[key]
        if section ~= nil then
            local schema = schemas[key]
            local component = entity[schema.add](entity)
            apply_setters(component, section, schema.setters)
        end
    end

    if prefab.lua_components then
        for _, class_name in ipairs(prefab.lua_components) do
            entity:add_lua_component(class_name)
        end
    end
end

---@param name string
---@param position? Vector2
---@param rotation_deg? number
---@param scale? Vector2
---@return Entity|nil
EntitySpawner.spawn_entity = function(name, position, rotation_deg, scale)
    local prefab = _G.__entity_prefab_registry[name]
    if not prefab then
        Debug.log_error("EntitySpawner.spawn_entity: prefab '" .. name .. "' is not registered")
        return nil
    end

    local entity = EntitySpawner.spawn_entity_c()

    apply_prefab(entity, prefab)

    local transform = entity:get_transform()
    transform:set_position(position or Vector2())
    transform:set_rotation((rotation_deg or 0) * Math.DEG_TO_RAD)
    transform:set_scale(scale or Vector2.one())

    return entity
end
