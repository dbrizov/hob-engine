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
        texture = Assets.RobotTexture,
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
        texture = Assets.RobotTexture,
    },
    lua_components = { "ContactLogger" },
}

DefineEntity.StaticCircle = {
    rigidbody = {},
    circle_collider = {
        collision_layer = Collision.Static,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
}

DefineEntity.DynamicCircle = {
    ticking = true,
    rigidbody = {
        body_type = BodyType.Dynamic,
    },
    circle_collider = {
        collision_layer = Collision.Dynamic,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic | Collision.Trigger,
    },
    sprite = {
        texture = Assets.RobotTexture,
    },
}

DefineEntity.TriggerCircle = {
    rigidbody = {},
    circle_collider = {
        trigger = true,
        collision_layer = Collision.Trigger,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
    sprite = {
        texture = Assets.RobotTexture,
        material = Material {
            shader = Assets.OutlineShader,
        },
    },
    lua_components = { "ContactLogger" },
}

DefineEntity.PulsingStaticCircle = {
    ticking = true,
    rigidbody = {},
    circle_collider = {
        collision_layer = Collision.Static,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
    sprite = {
        texture = Assets.RobotTexture,
        z_index = 1,
    },
    lua_components = { "ScalePulse" },
}

DefineEntity.PulsingTriggerCircle = {
    ticking = true,
    rigidbody = {},
    circle_collider = {
        trigger = true,
        collision_layer = Collision.Trigger,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
    sprite = {
        texture = Assets.RobotTexture,
        z_index = 1,
    },
    lua_components = { "ScalePulse", "ContactLogger" },
}
