-- Lightweight game-state singleton. Lets enemies and the spawner find the player and
-- track how many enemies are alive without a service locator (mirrors how the engine
-- exposes the Components / Entities globals).
Game = {
    player = nil,
    enemy_count = 0,
}

-- Spawn band, just inside the arena's perimeter walls (see main.lua arena layout).
Game.ARENA_HALF_WIDTH = 20.0
Game.ARENA_HALF_HEIGHT = 12.0

function Game.get_player()
    return Game.player
end

function Game.set_player(entity)
    Game.player = entity
end

function Game.on_enemy_spawned()
    Game.enemy_count = Game.enemy_count + 1
end

function Game.on_enemy_died()
    Game.enemy_count = math.max(0, Game.enemy_count - 1)
end
