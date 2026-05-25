DefineEntity.StaticBox = {
    rigidbody = {},
    box_collider = {
        collision_layer = Collision.Static,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
}

DefineEntity.DynamicBox = {
    ticking = true,
    rigidbody = {
        body_type = BodyType.Dynamic,
    },
    box_collider = {
        collision_layer = Collision.Dynamic,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic | Collision.Trigger,
    },
    sprite = {
        texture = "images/robot.png",
    },
}

DefineEntity.TriggerBox = {
    rigidbody = {},
    box_collider = {
        trigger = true,
        collision_layer = Collision.Trigger,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
    sprite = {
        texture = "images/robot.png",
    },
    lua_components = { "ContactLogger" },
}
