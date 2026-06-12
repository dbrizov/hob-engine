DefineComponent.Enemy = {
    __parent = Components.Character,
}
---@class Enemy : Character
local Enemy = Enemy

function Enemy:init()
    self.speed = 3.5
    self.max_health = 3
    self.health = self.max_health
    self.contact_damage = 1
    self.contact_range = 1.8       -- how close the enemy must be to hurt the player
    self.damage_interval = 0.8     -- seconds between contact hits
    self.damage_cooldown = 0.0
    self.flash_time = 0.0
    self.dead = false

    -- Obstacle avoidance: when wedged against cover, veer sideways to round it.
    self.avoid_timer = 0.0
    self.avoid_sign = 1.0
    self.avoid_angle = 1.0         -- radians to veer when blocked (~57 degrees)
    self.avoid_duration = 0.4
    self.blocked_speed_ratio = 0.5 -- below this fraction of full speed = blocked

    -- Ranged attack: slow, line-of-sight shots at the player.
    self.shoot_interval = 2.2
    self.shoot_cooldown = self.shoot_interval * math.random() -- stagger the first shot
    self.shoot_range = 13.0
    self.muzzle_offset = 1.5
end

function Enemy:enter_play()
    Game.on_enemy_spawned()

    -- Capture the base body tint so the hit flash can revert to it.
    self.material = self.entity:get_sprite():get_material()
    self.base_tint = self.material:get_tint()
end

function Enemy:physics_tick(fixed_delta_time)
    local player = Game.get_player()
    if player == nil then
        return
    end

    local body = self.entity:get_character_body()
    local to_player = player:get_transform():get_position() - body:get_position()
    if to_player:length_sqr() < 0.0001 then
        return
    end

    local direction = to_player:normalized()
    if self.avoid_timer > 0.0 then
        self.avoid_timer = self.avoid_timer - fixed_delta_time
        direction = Vector2.rotate_around(direction, Vector2.zero(), self.avoid_angle * self.avoid_sign)
    end

    self:move(direction, fixed_delta_time)

    -- move_and_slide reports the achieved velocity. If it's well below full speed we're
    -- wedged against cover, so pick a side and veer around it for a moment.
    local achieved_speed = body:get_velocity():length()
    if achieved_speed < self.speed * self.blocked_speed_ratio then
        if self.avoid_timer <= 0.0 then
            self.avoid_sign = (math.random() < 0.5) and -1.0 or 1.0
        end
        self.avoid_timer = self.avoid_duration
    end
end

function Enemy:tick(delta_time)
    self:update_facing()
    self:update_animation()
    self:update_contact_damage(delta_time)
    self:update_shooting(delta_time)
    self:update_flash(delta_time)
end

function Enemy:update_facing()
    local player = Game.get_player()
    if player == nil then
        return
    end

    local body = self.entity:get_character_body()
    local direction = player:get_transform():get_position() - body:get_position()
    body:set_rotation(math.atan(direction.y, direction.x))
end

function Enemy:update_animation()
    local animator = self.entity:get_sprite_animator()
    if animator == nil then
        return
    end

    local velocity = self.entity:get_character_body():get_velocity()
    local desired = velocity:length_sqr() > 0.01 and "run" or "idle"
    if animator:get_current_clip() ~= desired then
        animator:play(desired)
    end
end

function Enemy:update_contact_damage(delta_time)
    if self.damage_cooldown > 0.0 then
        self.damage_cooldown = self.damage_cooldown - delta_time
    end

    local player = Game.get_player()
    if player == nil then
        return
    end

    local body = self.entity:get_character_body()
    local distance = Vector2.distance(body:get_position(), player:get_transform():get_position())
    if distance <= self.contact_range and self.damage_cooldown <= 0.0 then
        local player_comp = player:get_lua_component(Components.Player)
        if player_comp ~= nil then
            player_comp:take_damage(self.contact_damage)
        end
        self.damage_cooldown = self.damage_interval
    end
end

function Enemy:update_shooting(delta_time)
    if self.shoot_cooldown > 0.0 then
        self.shoot_cooldown = self.shoot_cooldown - delta_time
    end

    local player = Game.get_player()
    if player == nil then
        return
    end

    local origin = self.entity:get_character_body():get_position()
    local to_player = player:get_transform():get_position() - origin
    local distance = to_player:length()
    if distance > self.shoot_range or self.shoot_cooldown > 0.0 then
        return
    end

    -- Only fire with a clear line of sight, so shots don't just hit our own cover.
    local hit = Physics.raycast(origin, to_player, distance, Collision.Static)
    if hit.hit then
        return
    end

    self:shoot(to_player)
    self.shoot_cooldown = self.shoot_interval
end

function Enemy:shoot(to_player)
    local forward = to_player:normalized()
    local origin = self.entity:get_character_body():get_position()
    local muzzle_pos = origin + forward * self.muzzle_offset
    local rotation = math.atan(forward.y, forward.x)
    EntitySpawner.spawn_entity(Entities.EnemyBullet, muzzle_pos, math.deg(rotation))
end

function Enemy:update_flash(delta_time)
    if self.flash_time <= 0.0 then
        return
    end

    self.flash_time = self.flash_time - delta_time
    if self.flash_time <= 0.0 then
        self.material:set_tint(self.base_tint)
    end
end

function Enemy:take_damage(amount)
    if self.dead then
        return
    end

    self.health = self.health - amount

    -- Brief red body tint for hit feedback (outline stays red).
    self.material:set_tint(Color(1.0, 0.6, 0.6, 1.0))
    self.flash_time = 0.2

    if self.health <= 0 then
        self.dead = true
        Game.on_enemy_died()
        EntitySpawner.destroy_entity(self.entity)
    end
end
