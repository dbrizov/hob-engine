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
--       lua_components = { Components.Player },
--   }

_G.__entity_prefab_registry = _G.__entity_prefab_registry or {}

-- Entities spawned from a prefab, tracked so hot reload can re-apply changed prefab data to them.
-- Strong list (an EntityRef's Lua lifetime != the entity's lifetime, so weak keys would
-- drop live entities); destroyed entities are pruned on each reload via EntityRef:is_valid().
_G.__spawned_entities = _G.__spawned_entities or {}

--- Assigning `DefineEntity.Foo = { ... }` registers a prefab usable via
--- `EntitySpawner.spawn_entity("Foo", ...)`. Recognized section keys are
--- `ticking`, `lua_components`, and one key per C++ component (e.g. `transform`,
--- `rigidbody`, `box_collider`, `sprite`, ...) bound via `bind_component_schema`
--- in lua_bind_components.cpp.
---@class DefineEntity
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

-- `Entities.Foo` evaluates to the prefab name string `"Foo"`. The registry only
-- exists so editors can autocomplete the name and catch typos via the LuaCATS
-- meta; there is no validation at index time (load order means a referenced
-- prefab may legitimately be defined later). Unknown names surface as
-- "prefab is not registered" errors at spawn time.
---@class Entities
_G.Entities = setmetatable({}, {
    __index = function(_, name) return name end,
})

local function apply_setters(component, section, setters)
    for prop, value in pairs(section) do
        local unwrapped = unwrap_def(value)
        local setter = setters[prop]
        if setter == nil then
            Debug.log_error("Unknown prefab property '" .. tostring(prop) .. "' for component")
        elseif type(setter) == "string" then
            component[setter](component, unwrapped)
        else
            setter(component, unwrapped)
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
        for _, entry in ipairs(prefab.lua_components) do
            entity:add_lua_component(entry)
        end
    end
end

-- Hot reload: re-apply a prefab's properties to an already-built entity.
-- Uses each component's getter (never its `add`) so no component is duplicated and no C++ init() re-fires.
-- Geometry/physics changes are diffed inside C++ setters.
-- Deliberately skipped: structure (add), the prefab's lua_components (refreshed by the metatable swap in hot_reload.lua),
-- and the transform's position/rotation/scale (spawn arguments, not prefab data).
local function reapply_prefab(entity, prefab)
    local schemas = _G.__component_schemas

    for _, key in ipairs(schemas.__order) do
        local section = prefab[key]
        if section ~= nil then
            local schema = schemas[key]
            local component = entity[schema.get](entity)
            if component ~= nil then
                apply_setters(component, section, schema.setters)
            end
        end
    end
end

-- Drop entries whose entity has been destroyed (single forward pass, preserves order).
local function compact_spawned_entities()
    local list = _G.__spawned_entities
    local n = 0
    for i = 1, #list do
        local entry = list[i]
        if entry.entity:is_valid() then
            n = n + 1
            list[n] = entry
        end
    end
    for i = #list, n + 1, -1 do
        list[i] = nil
    end
end

-- Re-applies current prefab data to every live spawned entity.
-- Called by hot_reload.lua after component classes have been rebuilt.
function _G.reapply_prefabs_to_spawned_entities()
    compact_spawned_entities()
    for _, entry in ipairs(_G.__spawned_entities) do
        local prefab = _G.__entity_prefab_registry[entry.prefab]
        if prefab then
            reapply_prefab(entry.entity, prefab)
        end
    end
end

-- Without reloads the spawn list never gets compacted, so bound its growth: compact once it
-- grows past a threshold, then set the next threshold to ~2x the live count (amortized O(1) per
-- spawn, list stays within ~2x the number of live spawned-from-prefab entities).
local spawned_compact_threshold = 64

local function track_spawned_entity(entity, name)
    local list = _G.__spawned_entities
    list[#list + 1] = { entity = entity, prefab = name }
    if #list >= spawned_compact_threshold then
        compact_spawned_entities()
        spawned_compact_threshold = math.max(64, #list * 2)
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
    track_spawned_entity(entity, name)

    local transform = entity:get_transform()
    transform:set_position(position or Vector2())
    transform:set_rotation((rotation_deg or 0) * Math.DEG_TO_RAD)
    transform:set_scale(scale or Vector2.one())

    return entity
end
