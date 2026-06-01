Cursor.config {
    -- texture = Assets.CursorTexture,
    pivot = Vector2(0.24, 0.1),
    scale = Vector2(0.8, 0.8)
}

local SCALE = 2.0
local ARENA_HALF_W = 10
local ARENA_HALF_H = 6
local S = Vector2(SCALE, SCALE)

-- Player at the center
EntitySpawner.spawn_entity("Player", Vector2(0.0, 0.0))

-- Perimeter walls
for x = -ARENA_HALF_W, ARENA_HALF_W do
    EntitySpawner.spawn_entity("StaticBox", Vector2(x * SCALE, ARENA_HALF_H * SCALE), 0, S)  -- top
    EntitySpawner.spawn_entity("StaticBox", Vector2(x * SCALE, -ARENA_HALF_H * SCALE), 0, S) -- bottom
end
for y = -ARENA_HALF_H + 1, ARENA_HALF_H - 1 do
    EntitySpawner.spawn_entity("StaticBox", Vector2(-ARENA_HALF_W * SCALE, y * SCALE), 0, S) -- left
    EntitySpawner.spawn_entity("StaticBox", Vector2(ARENA_HALF_W * SCALE, y * SCALE), 0, S)  -- right
end

-- Interior crates: two cover lines (top and bottom)
local crate_xs = { -3, -2, 2, 3 }
for _, x in ipairs(crate_xs) do
    EntitySpawner.spawn_entity("StaticBox", Vector2(x * SCALE, 2.0 * SCALE), 0, S)
    EntitySpawner.spawn_entity("StaticBox", Vector2(x * SCALE, -2.0 * SCALE), 0, S)
end

-- Corner pillars (circles)
EntitySpawner.spawn_entity("StaticCircle", Vector2(-7.0 * SCALE, 4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity("StaticCircle", Vector2(7.0 * SCALE, 4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity("StaticCircle", Vector2(-7.0 * SCALE, -4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity("StaticCircle", Vector2(7.0 * SCALE, -4.0 * SCALE), 0, S)

-- Mid-line pillars (gives the center extra cover to weave around)
EntitySpawner.spawn_entity("StaticCircle", Vector2(-5.0 * SCALE, 0.0), 0, S)
EntitySpawner.spawn_entity("StaticCircle", Vector2(5.0 * SCALE, 0.0), 0, S)

-- Checkpoint triggers (top and bottom of the arena)
EntitySpawner.spawn_entity("TriggerCircle", Vector2(0.0, 4.0 * SCALE), 0, S)
EntitySpawner.spawn_entity("TriggerCircle", Vector2(0.0, -4.0 * SCALE), 0, S)

-- Live-rescale test: a box that pulses its scale with a sin wave.
-- Walk into it to verify the player gets pushed out as it grows.
EntitySpawner.spawn_entity("PulsingStaticCircle", Vector2(-4.0, 0.0))
EntitySpawner.spawn_entity("PulsingTriggerCircle", Vector2(4.0, 0.0))

-- Dynamic entities for collision testing. With Y-up gravity these fall
-- and pile up on the bottom wall and crates.
-- local dyn_y = (ARENA_HALF_H - 1) * SCALE
-- for x = -8, 8, 2 do
--     EntitySpawner.spawn_entity("DynamicBox", Vector2(x, dyn_y))
--     EntitySpawner.spawn_entity("DynamicCircle", Vector2(x + 1.0, dyn_y - 2.0))
-- end
