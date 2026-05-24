local collision = require("collision")

local M = {}

local function spawner()
    return app:get_entity_spawner()
end

local function assets()
    return app:get_assets()
end

function M.player(position)
    local entity = spawner():spawn_entity()
    entity:set_ticking(true)
    entity:get_transform():set_position(position)

    entity:add_input()
    entity:add_lua_script("components.player")

    local character_body = entity:add_character_body()
    character_body:set_collision_layer(collision.KINEMATIC)
    character_body:set_collision_mask(collision.STATIC | collision.DYNAMIC | collision.TRIGGER)
    character_body:set_solver_ignore_mask(collision.TRIGGER)

    local capsule_collider = entity:get_capsule_collider()
    capsule_collider:set_capsule(Capsule(Vector2.zero(), Vector2.zero(), 1.2))

    local sprite = entity:add_sprite()
    sprite:set_texture_id(assets():load_texture("images/player/HJ_run01.png"))
    sprite:set_z_index(1)

    return entity
end

function M.static_box(position, rotation_degrees)
    local entity = spawner():spawn_entity()
    entity:get_transform():set_position(position)
    entity:get_transform():set_rotation(rotation_degrees * Math.DEG_TO_RAD)

    local rigidbody = entity:add_rigidbody()
    rigidbody:set_body_type(BodyType.Static)

    local box = entity:add_box_collider()
    box:set_collision_layer(collision.STATIC)
    box:set_collision_mask(collision.STATIC | collision.DYNAMIC | collision.KINEMATIC)

    return entity
end

function M.dynamic_box(position, rotation_degrees)
    local entity = spawner():spawn_entity()
    entity:set_ticking(true)
    entity:get_transform():set_position(position)
    entity:get_transform():set_rotation(rotation_degrees * Math.DEG_TO_RAD)

    local rigidbody = entity:add_rigidbody()
    rigidbody:set_body_type(BodyType.Dynamic)

    local box = entity:add_box_collider()
    box:set_collision_layer(collision.DYNAMIC)
    box:set_collision_mask(collision.STATIC | collision.DYNAMIC | collision.KINEMATIC | collision.TRIGGER)

    local sprite = entity:add_sprite()
    sprite:set_texture_id(assets():load_texture("images/robot.png"))

    return entity
end

function M.trigger_box(position, rotation_degrees)
    local entity = spawner():spawn_entity()
    entity:get_transform():set_position(position)
    entity:get_transform():set_rotation(rotation_degrees * Math.DEG_TO_RAD)

    entity:add_rigidbody()

    local box = entity:add_box_collider()
    box:set_trigger(true)
    box:set_collision_layer(collision.TRIGGER)
    box:set_collision_mask(collision.STATIC | collision.DYNAMIC | collision.KINEMATIC)

    local sprite = entity:add_sprite()
    sprite:set_texture_id(assets():load_texture("images/robot.png"))

    entity:add_lua_script("components.contact_logger")

    return entity
end

return M
