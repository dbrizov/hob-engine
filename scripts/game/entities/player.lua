DefineEntity.Player = {
    ticking = true,
    transform = {
        interpolate_physics = false,
    },
    input = {},
    character_body = {
        collision_layer = Collision.Kinematic,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Trigger,
        solver_ignore_mask = Collision.Trigger,
        capsule = Capsule(Vector2.zero(), Vector2.zero(), 1.2),
    },
    sprite = {
        texture = Assets.PlayerTexture,
        z_index = 2,
    },
    lua_components = { "Player", "ContactLogger" },
}
