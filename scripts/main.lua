Cursor.config {
    texture = Assets.CursorTexture,
    pivot = Vector2(0.24, 0.1),
    scale = Vector2(0.8, 0.8)
}

EntitySpawner.spawn_entity("Player", Vector2(0.0, 0.0))

-- floor
for x = -4, 4 do
    EntitySpawner.spawn_entity("StaticBox", Vector2(x, -3.0))
end

-- stairs
local stair_heights = { -2.7, -2.4, -2.1, -1.8, -1.5, -1.2, -0.9, -0.6, -0.3 }
for i, y in ipairs(stair_heights) do
    EntitySpawner.spawn_entity("StaticBox", Vector2(4 + i, y))
end

-- left wall
for y = -3, 4 do
    EntitySpawner.spawn_entity("StaticBox", Vector2(-5.0, y))
end

-- trigger boxes
EntitySpawner.spawn_entity("TriggerBox", Vector2(0.0, 2.0))

-- dynamic boxes
EntitySpawner.spawn_entity("DynamicBox", Vector2(-3.0, 3.0), 60.0)
EntitySpawner.spawn_entity("DynamicBox", Vector2(-2.0, 0.0), 0.0)

-- dynamic circles
EntitySpawner.spawn_entity("DynamicCircle", Vector2(7.25, 2.0))
EntitySpawner.spawn_entity("DynamicCircle", Vector2(6.5, 4.0))
EntitySpawner.spawn_entity("DynamicCircle", Vector2(10.3, 4.0))
