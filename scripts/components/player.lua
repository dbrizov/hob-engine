local Player = {}
Player.__index = Player

function Player.new()
    local self = setmetatable({}, Player)
    self.speed = 7.0
    self.camera_follow_speed = 10.0
    self.movement_input = Vector2.zero()
    self.x_axis_id = nil
    self.y_axis_id = nil
    self.slow_motion_action_id = nil
    return self
end

function Player:enter_play()
    local input = self.entity:get_input()

    self.x_axis_id = input:bind_axis("horizontal", function(axis)
        self.movement_input.x = axis
    end)

    self.y_axis_id = input:bind_axis("vertical", function(axis)
        self.movement_input.y = axis
    end)

    self.slow_motion_action_id = input:bind_action("slow_motion", InputEventType.Pressed, function()
        local new_scale = Timer.get_time_scale() < 1.0 and 1.0 or 0.2
        Timer.set_time_scale(new_scale)
    end)
end

function Player:exit_play()
    local input = self.entity:get_input()
    input:unbind_axis("horizontal", self.x_axis_id)
    input:unbind_axis("vertical", self.y_axis_id)
    input:unbind_action("slow_motion", self.slow_motion_action_id)
end

function Player:physics_tick(fixed_delta_time)
    local movement = self.movement_input
    if movement:length_sqr() > 1.0 then
        movement = movement:normalized()
    end

    local velocity = movement * self.speed
    local character_body = self.entity:get_character_body()
    character_body:move_and_slide(velocity, fixed_delta_time)

    local position = self.entity:get_transform():get_position()
    self:update_camera_position(position, fixed_delta_time)
    self:update_rotation(fixed_delta_time)
end

function Player:update_camera_position(target_position, delta_time)
    local current_position = Camera.get_position()
    local new_position = Vector2.lerp(current_position, target_position, delta_time * self.camera_follow_speed)
    Camera.set_position(new_position)
end

function Player:update_rotation(delta_time)
    local mouse_screen = Input.get_mouse_screen_position()
    local mouse_world = Camera.screen_to_world(mouse_screen)

    local character_body = self.entity:get_character_body()
    local player_pos = character_body:get_position()
    local direction = mouse_world - player_pos

    local radians = math.atan(direction.y, direction.x)
    character_body:set_rotation(radians)
end

function Player:on_collision_enter(other)
    log("collision_enter: " .. other:get_entity():get_id())
end

function Player:on_collision_exit(other)
    log("collision_exit: " .. other:get_entity():get_id())
end

function Player:on_trigger_enter(other)
    log("trigger_enter: " .. other:get_entity():get_id())
end

function Player:on_trigger_exit(other)
    log("trigger_exit: " .. other:get_entity():get_id())
end

return Player
