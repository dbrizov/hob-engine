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

-- Maps entity id -> the prefab name it was spawned from, so hot reload can re-apply changed prefab data to live entities.
_G.__entity_prefab_by_id = _G.__entity_prefab_by_id or {}

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

-- Sentinel so a captured default of `nil` (e.g. an unset texture) survives in a table,
-- where a raw nil value would just remove the key.
local NIL_DEFAULT = {}

-- Hot reload: re-apply a prefab's properties to an already-built entity. The prefab is the
-- source of truth: within a declared section, a present field is applied and an *absent* field is
-- reset to the component's default (read once per type from a throwaway probe via get_defaults).
-- Top-level `ticking` is always governed -- absent means the Entity default (not ticking).
-- Uses each component's getter (never its `add`) so no component is duplicated and no C++ init() re-fires.
-- Deliberately skipped: structure (no add/remove) and sections the prefab does not declare (the
-- component may be owned by another, e.g. CharacterBody configures a Rigidbody, or added at runtime);
-- the prefab's lua_components (refreshed by the metatable swap in hot_reload.lua); and the transform's
-- position/rotation/scale (spawn arguments, not prefab data).
local function reapply_prefab(entity, prefab, get_defaults)
    local schemas = _G.__component_schemas

    local ticking = prefab.ticking
    if ticking == nil then
        ticking = false
    end
    entity:set_ticking(ticking)

    for _, key in ipairs(schemas.__order) do
        local section = prefab[key]
        if section ~= nil then
            local schema = schemas[key]
            local component = entity[schema.get](entity)
            if component ~= nil then
                local defaults = get_defaults(key)
                for field, setter in pairs(schema.setters) do
                    local value
                    if section[field] ~= nil then
                        value = unwrap_def(section[field])
                    else
                        value = defaults[field]
                        if value == NIL_DEFAULT then
                            value = nil
                        end
                    end

                    if type(setter) == "string" then
                        component[setter](component, value)
                    else
                        setter(component, value)
                    end
                end
            end
        end
    end
end

-- Re-applies current prefab data to every live spawned entity (called by hot_reload.lua).
-- Rebuilds the id->prefab map from the live set as it goes, dropping ids of entities that were
-- destroyed outside the Lua wrapper (e.g. by C++ spawning/destroying directly).
function _G.__reapply_prefabs_to_spawned_entities()
    local schemas = _G.__component_schemas

    -- Per-component-type default snapshots, read lazily from throwaway probe entities. Probes stay
    -- alive for the whole pass so reference-returning getters (e.g. get_material) remain valid while
    -- their values are applied, then are destroyed at the end. Nothing persists, so no TextureRefs or
    -- clips are pinned alive past the reload.
    local defaults_cache = {}
    local probes = {}

    local function get_defaults(key)
        local cached = defaults_cache[key]
        if cached then
            return cached
        end

        local schema = schemas[key]
        local probe = EntitySpawner.spawn_entity_c()
        probes[#probes + 1] = probe

        local component = probe[schema.add](probe)
        local defaults = {}
        if component ~= nil then
            for field, getter in pairs(schema.getters) do
                local v = component[getter](component)
                if v == nil then
                    v = NIL_DEFAULT
                end
                defaults[field] = v
            end
        end

        defaults_cache[key] = defaults
        return defaults
    end

    local live = {}
    EntitySpawner.for_each(function(entity)
        local id = entity:get_id()
        local name = _G.__entity_prefab_by_id[id]
        if name then
            live[id] = name
            local prefab = _G.__entity_prefab_registry[name]
            if prefab then
                reapply_prefab(entity, prefab, get_defaults)
            end
        end
    end)
    _G.__entity_prefab_by_id = live

    -- Probes are pending (never entered play), so this drops them from the spawn queue synchronously.
    for _, probe in ipairs(probes) do
        EntitySpawner.destroy_entity_c(probe)
    end
end

-- Wrap spawn so a prefab name resolves to a fully-built entity (spawn_entity_c is the raw C++ spawn).
local spawn_entity_c = EntitySpawner.spawn_entity_c

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

    local entity = spawn_entity_c()

    apply_prefab(entity, prefab)
    _G.__entity_prefab_by_id[entity:get_id()] = name

    local transform = entity:get_transform()
    transform:set_position(position or Vector2())
    transform:set_rotation((rotation_deg or 0) * Math.DEG_TO_RAD)
    transform:set_scale(scale or Vector2.one())

    return entity
end

-- Wrap destroy to release the entity's id->prefab entry (destroy_entity_c is the raw C++ destroy).
local destroy_entity_c = EntitySpawner.destroy_entity_c

---@param entity Entity
EntitySpawner.destroy_entity = function(entity)
    if entity then
        _G.__entity_prefab_by_id[entity:get_id()] = nil
    end
    destroy_entity_c(entity)
end
