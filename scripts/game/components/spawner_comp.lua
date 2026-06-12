DefineComponent.EnemySpawner = {}
---@class EnemySpawner : LuaComponent
local EnemySpawner = EnemySpawner

function EnemySpawner:init()
    self.max_alive = 8
    self.spawn_interval = 1.5
    self.spawn_timer = self.spawn_interval
    self.min_player_distance = 6.0   -- don't spawn right on top of the player
end

function EnemySpawner:tick(delta_time)
    self.spawn_timer = self.spawn_timer - delta_time
    if self.spawn_timer > 0.0 then
        return
    end
    self.spawn_timer = self.spawn_interval

    if Game.enemy_count >= self.max_alive then
        return
    end

    self:spawn_enemy()
end

function EnemySpawner:spawn_enemy()
    local position = self:pick_spawn_position()
    EntitySpawner.spawn_entity(Entities.Enemy, position)
end

function EnemySpawner:pick_spawn_position()
    local player = Game.get_player()
    local player_pos = player ~= nil and player:get_transform():get_position() or Vector2.zero()

    -- A few attempts to land a perimeter point that isn't on top of the player.
    local position = self:random_perimeter_position()
    for _ = 1, 4 do
        if Vector2.distance(position, player_pos) >= self.min_player_distance then
            break
        end
        position = self:random_perimeter_position()
    end

    return position
end

function EnemySpawner:random_perimeter_position()
    local hw = Game.ARENA_HALF_WIDTH
    local hh = Game.ARENA_HALF_HEIGHT
    local edge = math.random(4)

    if edge == 1 then
        return Vector2(math.random() * 2.0 * hw - hw, hh)   -- top
    elseif edge == 2 then
        return Vector2(math.random() * 2.0 * hw - hw, -hh)  -- bottom
    elseif edge == 3 then
        return Vector2(-hw, math.random() * 2.0 * hh - hh)  -- left
    else
        return Vector2(hw, math.random() * 2.0 * hh - hh)   -- right
    end
end
