DefineEntity.PlayerBullet = {
    ticking = true,
    rigidbody = {
        body_type = BodyType.Kinematic,
    },
    box_collider = {
        trigger = true,
        collision_layer = Collision.Bullet,
        collision_mask = Collision.Static | Collision.Kinematic,
        aabb = AABB(Vector2.zero(), Vector2(0.15, 0.15)),
    },
    sprite = {
        texture = Textures.Rectangle,
        material = Materials.PlayerBullet,
        scale = Vector2(0.3, 0.3),
        z_index = 1,
    },
    lua_components = { Components.PlayerBullet },
}

DefineEntity.EnemyBullet = {
    ticking = true,
    rigidbody = {
        body_type = BodyType.Kinematic,
    },
    box_collider = {
        trigger = true,
        collision_layer = Collision.EnemyBullet,
        collision_mask = Collision.Static | Collision.Kinematic,
        aabb = AABB(Vector2.zero(), Vector2(0.15, 0.15)),
    },
    sprite = {
        texture = Textures.Rectangle,
        material = Materials.EnemyBullet,
        scale = Vector2(0.3, 0.3),
        z_index = 1,
    },
    lua_components = { Components.EnemyBullet },
}
