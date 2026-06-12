DefineEntity.StaticBox = {
    rigidbody = {},
    box_collider = {
        collision_layer = Collision.Static,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic | Collision.Bullet | Collision.EnemyBullet,
    },
    -- The 64x64 white square at 64 px/m is exactly 1 m, so sprite scale 1 matches the
    -- default 1 m box collider at any entity scale.
    sprite = {
        texture = Textures.Rectangle,
        material = Materials.Wall,
        z_index = -1,
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
}

DefineEntity.TriggerBox = {
    rigidbody = {},
    box_collider = {
        trigger = true,
        collision_layer = Collision.Trigger,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
    -- lua_components = { Components.ContactLogger },
}

DefineEntity.StaticCircle = {
    rigidbody = {},
    circle_collider = {
        collision_layer = Collision.Static,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic | Collision.Bullet | Collision.EnemyBullet,
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
}

DefineEntity.TriggerCircle = {
    rigidbody = {},
    circle_collider = {
        trigger = true,
        collision_layer = Collision.Trigger,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Kinematic,
    },
    -- lua_components = { Components.ContactLogger },
}
