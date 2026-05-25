-- DefineEntity: declarative entity templates.
--
-- Usage:
--   DefineEntity.Player = {
--       ticking = true,
--       character_body = {
--           collision_layer = Collision.KINEMATIC,
--           collision_mask  = Collision.STATIC | Collision.DYNAMIC | Collision.TRIGGER,
--           capsule         = Capsule(Vector2.zero(), Vector2.zero(), 1.2),
--       },
--       sprite = { texture = "images/player/HJ_run01.png", z_index = 1 },
--       input = {},
--       lua_components = { "Player" },
--   }
--
--   local player = EntitySpawner.spawn_entity("Player")
--   player:get_transform():set_position(Vector2(5, 0))

_G.__entity_prefab_registry = _G.__entity_prefab_registry or {}

_G.DefineEntity = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            log_error("DefineEntity." .. tostring(name) .. " must be assigned a table")
            return
        end

        _G.__entity_prefab_registry[name] = def
    end,
    __index = function(_, name)
        return _G.__entity_prefab_registry[name]
    end,
})

local function apply_setters(component, entity, section, setters)
    for prop, value in pairs(section) do
        local setter = setters[prop]
        if setter == nil then
            log_error("Unknown prefab property '" .. tostring(prop) .. "' for component")
        elseif type(setter) == "string" then
            component[setter](component, value)
        else
            setter(component, value, entity)
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
            apply_setters(component, entity, section, schema.setters)
        end
    end

    if prefab.lua_components then
        for _, class_name in ipairs(prefab.lua_components) do
            entity:add_lua_component(class_name)
        end
    end
end

EntitySpawner.spawn_entity = function(name, position, rotation_degrees)
    local entity = EntitySpawner.spawn_entity_c()
    if name == nil then
        return entity
    end

    if type(name) ~= "string" then
        log_error("EntitySpawner.spawn_entity expects a prefab name string or no argument")
        return entity
    end

    local prefab = _G.__entity_prefab_registry[name]
    if not prefab then
        log_error("EntitySpawner.spawn_entity: prefab '" .. name .. "' is not registered")
        return entity
    end

    apply_prefab(entity, prefab)

    local transform = entity:get_transform()
    transform:set_position(position or Vector2())
    transform:set_rotation((rotation_degrees or 0) * Math.DEG_TO_RAD)

    return entity
end
