local spawn = require("spawn")

spawn.player(Vector2(0.0, 0.0))

-- floor
for x = -4, 4 do
    spawn.static_box(Vector2(x, -3.0), 0.0)
end

-- stairs
local stair_heights = { -2.7, -2.4, -2.1, -1.8, -1.5, -1.2, -0.9, -0.6, -0.3 }
for i, y in ipairs(stair_heights) do
    spawn.static_box(Vector2(4 + i, y), 0.0)
end

-- left wall
for y = -3, 4 do
    spawn.static_box(Vector2(-5.0, y), 0.0)
end

-- dynamic boxes
-- spawn.dynamic_box(Vector2(-3.0, 3.0), 60.0)
-- spawn.dynamic_box(Vector2(-2.0, 0.0), 0.0)
-- spawn.dynamic_box(Vector2(-1.0, 1.5), 30.0)
-- spawn.dynamic_box(Vector2(0.0, 3.0), 40.0)
-- spawn.dynamic_box(Vector2(1.0, 4.5), 0.0)
-- spawn.dynamic_box(Vector2(2.0, 6.0), -10.0)

-- trigger boxes
spawn.trigger_box(Vector2(0.0, 2.0), 0.0)
