-- Stunnable: simple stun-timer behavior.
-- The host component should call tick_stun(dt) from its tick().

DefineMixin.Stunnable = {}
---@class Stunnable
---@field stun_timer number?
local Stunnable = Stunnable

function Stunnable:apply_stun(duration)
    self.stun_timer = duration
    log(self.class_name .. ": stunned for " .. duration .. "s")
end

function Stunnable:is_stunned()
    return (self.stun_timer or 0) > 0
end

function Stunnable:tick_stun(dt)
    if self.stun_timer and self.stun_timer > 0 then
        self.stun_timer = self.stun_timer - dt
    end
end

-- Uncomment to verify the engine reports a key collision against
-- Damageable.is_dead when both mixins are listed in __mixins.
-- Expect a log_error: "mixin 'Stunnable' key 'is_dead' collides ..."
--
-- function Stunnable:is_dead()
--     log("Stunnable:is_dead()")
--     return false
-- end
