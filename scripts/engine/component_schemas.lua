local schemas = {
    rigidbody = {
        add = "add_rigidbody",
        setters = {
            body_type = "set_body_type",
            fixed_rotation = "set_fixed_rotation",
        },
    },

    character_body = {
        add = "add_character_body",
        setters = {
            collision_layer = "set_collision_layer",
            collision_mask = "set_collision_mask",
            solver_ignore_mask = "set_solver_ignore_mask",
            capsule = function(comp, value) comp:get_entity():get_capsule_collider():set_capsule(value) end,
        },
    },

    box_collider = {
        add = "add_box_collider",
        setters = {
            aabb = "set_aabb",
            density = "set_density",
            friction = "set_friction",
            bounciness = "set_bounciness",
            collision_layer = "set_collision_layer",
            collision_mask = "set_collision_mask",
            trigger = "set_trigger",
        },
    },

    capsule_collider = {
        add = "add_capsule_collider",
        setters = {
            capsule = "set_capsule",
            density = "set_density",
            friction = "set_friction",
            bounciness = "set_bounciness",
            collision_layer = "set_collision_layer",
            collision_mask = "set_collision_mask",
            trigger = "set_trigger",
        },
    },

    sprite = {
        add = "add_sprite",
        setters = {
            texture = function(comp, value) comp:set_texture_id(Assets.load_texture(value)) end,
            pivot = "set_pivot",
            scale = "set_scale",
            tint = "set_tint",
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
