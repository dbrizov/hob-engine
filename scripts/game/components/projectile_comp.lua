-- Shared projectile behaviour. Subclasses set `target_class` (the component it damages)
-- and `friendly_class` (the shooter's team, which it passes through). Travel, lifetime
-- and impact are shared.
DefineComponent.Projectile = {}
---@class Projectile : LuaComponent
local Projectile = Projectile

function Projectile:init()
    self.speed = 24.0
    self.lifetime = 2.0
    self.damage = 1
    self.dead = false
end

function Projectile:enter_play()
    -- The spawn rotation encodes the fire direction; convert it to a velocity once.
    local rotation = self.entity:get_transform():get_rotation()
    local forward = Vector2(math.cos(rotation), math.sin(rotation))
    self.entity:get_rigidbody():set_velocity(forward * self.speed)
end

function Projectile:tick(delta_time)
    self.lifetime = self.lifetime - delta_time
    if self.lifetime <= 0.0 then
        self:destroy()
    end
end

function Projectile:on_trigger_enter(other)
    if self.dead then
        return
    end

    local other_entity = other:get_entity()

    local target = other_entity:get_lua_component(self.target_class)
    if target ~= nil then
        target:take_damage(self.damage)
        self:destroy()
        return
    end

    -- Pass through the shooter's own team; destroy on walls / cover.
    if other_entity:get_lua_component(self.friendly_class) ~= nil then
        return
    end

    self:destroy()
end

function Projectile:destroy()
    if self.dead then
        return
    end
    self.dead = true
    EntitySpawner.destroy_entity(self.entity)
end

-- Player's shots: damage enemies, pass through the player.
DefineComponent.PlayerBullet = {
    __parent = Components.Projectile,
    target_class = Components.Enemy,
    friendly_class = Components.Player,
}

-- Enemy shots: damage the player, pass through enemies. Slower than the player's.
DefineComponent.EnemyBullet = {
    __parent = Components.Projectile,
    target_class = Components.Player,
    friendly_class = Components.Enemy,
}
---@class EnemyBullet : Projectile
local EnemyBullet = EnemyBullet

function EnemyBullet:init()
    Projectile.init(self)
    self.speed = 14.0
end
