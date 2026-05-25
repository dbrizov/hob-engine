-- component_schemas: maps prefab keys to Entity:add_* methods and per-property setters.
--
-- Schema entry shape:
--   {
--       add = "add_method_name",
--       setters = {
--           prop_name = "set_method_name",         -- simple setter
--           prop_name = function(c, v, entity)     -- custom setter; can touch sibling components
--               ...
--           end,
--       },
--   }
--
-- Iteration order is fixed (see __order below) so order-sensitive Box2D setup
-- (bodies before colliders, etc.) is deterministic.

local schemas = {
    rigidbody = {
        add = "add_rigidbody",
        setters = {
            body_type      = "set_body_type",
            fixed_rotation = "set_fixed_rotation",
        },
    },

    character_body = {
        add = "add_character_body",
        setters = {
            collision_layer    = "set_collision_layer",
            collision_mask     = "set_collision_mask",
            solver_ignore_mask = "set_solver_ignore_mask",
            capsule            = function(_, v, entity) entity:get_capsule_collider():set_capsule(v) end,
        },
    },

    box_collider = {
        add = "add_box_collider",
        setters = {
            aabb            = "set_aabb",
            density         = "set_density",
            friction        = "set_friction",
            bounciness      = "set_bounciness",
            collision_layer = "set_collision_layer",
            collision_mask  = "set_collision_mask",
            trigger         = "set_trigger",
        },
    },

    capsule_collider = {
        add = "add_capsule_collider",
        setters = {
            capsule         = "set_capsule",
            density         = "set_density",
            friction        = "set_friction",
            bounciness      = "set_bounciness",
            collision_layer = "set_collision_layer",
            collision_mask  = "set_collision_mask",
            trigger         = "set_trigger",
        },
    },

    sprite = {
        add = "add_sprite",
        setters = {
            texture = function(c, v) c:set_texture_id(Assets.load_texture(v)) end,
            pivot   = "set_pivot",
            scale   = "set_scale",
            tint    = "set_tint",
            z_index = "set_z_index",
        },
    },

    input = {
        add = "add_input",
        setters = {},
    },

    camera = {
        add = "add_camera",
        setters = {},
    },
}

-- Order in which prefab keys are processed.
schemas.__order = {
    "rigidbody",
    "character_body",
    "box_collider",
    "capsule_collider",
    "sprite",
    "input",
    "camera",
}

_G.__component_schemas = schemas
