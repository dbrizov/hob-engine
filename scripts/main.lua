if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end

local player = EntitySpawner.spawn_entity("Player", Vector2(0.0, 0.0))
local lua_components = player:get_lua_components()
log(player)

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

-- dynamic boxes
EntitySpawner.spawn_entity("DynamicBox", Vector2(-3.0, 3.0), 60.0)
EntitySpawner.spawn_entity("DynamicBox", Vector2(-2.0, 0.0), 0.0)

-- trigger boxes
EntitySpawner.spawn_entity("TriggerBox", Vector2(0.0, 2.0))
