---@enum Collision
Collision = {
    None = 0,
    Static = 1 << 0,
    Dynamic = 1 << 1,
    Kinematic = 1 << 2,
    Trigger = 1 << 3,
    Bullet = 1 << 4,       -- player's shots
    EnemyBullet = 1 << 5,  -- enemies' shots
}
