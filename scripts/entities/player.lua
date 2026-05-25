DefineEntity.Player = {
    ticking = true,
    input = {},
    character_body = {
        collision_layer = Collision.Kinematic,
        collision_mask = Collision.Static | Collision.Dynamic | Collision.Trigger,
        solver_ignore_mask = Collision.Trigger,
        capsule = Capsule(Vector2.zero(), Vector2.zero(), 1.2),
    },
    sprite = {
        texture = "images/player/HJ_run01.png",
        z_index = 1,
    },
    lua_components = { "Player" },
}
