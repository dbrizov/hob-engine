DefineComponent.Weapon = {}
---@class Weapon : LuaComponent
local Weapon = Weapon

function Weapon:init()
    self.fire_rate = 8.0       -- shots per second while the button is held
    self.muzzle_offset = 1.5   -- spawn distance ahead of the player (clears the body)
    self.fire_cooldown = 0.0
end

function Weapon:tick(delta_time)
    if self.fire_cooldown > 0.0 then
        self.fire_cooldown = self.fire_cooldown - delta_time
    end

    if Input.is_mouse_button_down(0) and self.fire_cooldown <= 0.0 then
        self:fire()
        self.fire_cooldown = 1.0 / self.fire_rate
    end
end

function Weapon:fire()
    local origin = self.entity:get_transform():get_position()
    local mouse_world = Camera.screen_to_world(Input.get_mouse_screen_position())

    local direction = mouse_world - origin
    if direction:length_sqr() < 0.0001 then
        direction = Vector2.right()
    end
    local forward = direction:normalized()

    local muzzle_pos = origin + forward * self.muzzle_offset
    local rotation = math.atan(forward.y, forward.x)
    EntitySpawner.spawn_entity(Entities.PlayerBullet, muzzle_pos, math.deg(rotation))
end
