Cursor.set_texture(unwrap_def(Textures.Cursor))
Cursor.get_material():set_outline_color(Color.black())
Cursor.get_material():set_outline_width(1.0)
Cursor.set_pivot(Vector2(0.2, 0.1))

local SCALE = 2.0
local ARENA_HALF_W = 11
local ARENA_HALF_H = 7
local S = Vector2(SCALE, SCALE)

-- Camera (owns world-to-screen scale). Must be spawned before any rendering.
EntitySpawner.spawn_entity(Entities.Camera, Vector2(0.0, 0.0))

-- Player at the center
EntitySpawner.spawn_entity(Entities.Player, Vector2(0.0, 0.0))

-- Enemy spawner: drives the endless survival waves that chase the player.
EntitySpawner.spawn_entity(Entities.EnemySpawner, Vector2(0.0, 0.0))

-- Perimeter walls: a single-tile-thick closed rectangle of impassable static tiles.
for x = -ARENA_HALF_W, ARENA_HALF_W do
    EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(x * SCALE, ARENA_HALF_H * SCALE), 0, S)   -- top
    EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(x * SCALE, -ARENA_HALF_H * SCALE), 0, S)  -- bottom
end
for y = -ARENA_HALF_H + 1, ARENA_HALF_H - 1 do
    EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(-ARENA_HALF_W * SCALE, y * SCALE), 0, S)  -- left
    EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(ARENA_HALF_W * SCALE, y * SCALE), 0, S)   -- right
end

-- Interior crates: two cover lines (top and bottom)
local crate_xs = { -3, -2, 2, 3 }
for _, x in ipairs(crate_xs) do
    EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(x * SCALE, 2.0 * SCALE), 0, S)
    EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(x * SCALE, -2.0 * SCALE), 0, S)
end

-- Corner pillars (boxes)
EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(-7.0 * SCALE, 4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(7.0 * SCALE, 4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(-7.0 * SCALE, -4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(7.0 * SCALE, -4.0 * SCALE), 0, S)

-- Mid-line pillars (gives the center extra cover to weave around)
EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(-5.0 * SCALE, 0.0), 0, S)
EntitySpawner.spawn_entity(Entities.StaticBox, Vector2(5.0 * SCALE, 0.0), 0, S)
