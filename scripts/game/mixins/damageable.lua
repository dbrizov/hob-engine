-- Damageable: HP + damage/heal helpers.
-- The host component is responsible for initializing self.hp in its init().

DefineMixin.Damageable = {}
---@class Damageable
---@field hp number
local Damageable = Damageable

function Damageable:take_damage(amount)
    self.hp = (self.hp or 0) - amount
    Debug.log(self.class_name .. ": took " .. amount .. " damage, hp=" .. self.hp)
end

function Damageable:heal(amount)
    self.hp = (self.hp or 0) + amount
    Debug.log(self.class_name .. ": healed " .. amount .. ", hp=" .. self.hp)
end

function Damageable:is_dead()
    return (self.hp or 0) <= 0
end
