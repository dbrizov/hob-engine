DefineComponent.PulseScale = {}
---@class PulseScale : LuaComponent
local PulseScale = PulseScale

function PulseScale:init()
    self.min_scale = 1.0
    self.max_scale = 3.0
    self.speed = 2.0 -- radians/sec
    self.time = 0.0
end

function PulseScale:tick(delta_time)
    self.time = self.time + delta_time

    local t = (math.sin(self.time * self.speed) + 1.0) * 0.5
    local s = Math.lerp(self.min_scale, self.max_scale, t)

    self.entity:get_transform():set_scale(Vector2(s, s))
end
