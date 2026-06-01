DefineComponent.ZoomPulse = {}
---@class ZoomPulse : LuaComponent
local ZoomPulse = ZoomPulse

function ZoomPulse:init()
    self.min_zoom = 0.5
    self.max_zoom = 2.0
    self.speed = 1.0 -- radians/sec
    self.time = 0.0
end

function ZoomPulse:tick(delta_time)
    self.time = self.time + delta_time

    local t = (math.sin(self.time * self.speed) + 1.0) * 0.5
    local zoom = Math.lerp(self.min_zoom, self.max_zoom, t)

    Camera.set_zoom(zoom)
end
