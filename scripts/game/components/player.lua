DefineComponent.Player = {
    __parent = "Character",
    __mixins = { "Damageable", "Stunnable" },
}
---@class Player : Character, Damageable, Stunnable
local Player = Player

function Player:init()
    self.speed = 7.0
    self.camera_follow_speed = 10.0
    self.movement_input = Vector2.zero()
    self.x_axis_id = nil
    self.y_axis_id = nil
    self.slow_motion_action_id = nil

    -- Damageable state.
    self.hp = 100
    -- Stunnable state.
    self.stun_timer = 0
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
    self:move(self.movement_input, fixed_delta_time)

    local position = self.entity:get_transform():get_position()
    self:update_camera_position(position, fixed_delta_time)
    self:update_rotation(fixed_delta_time)
end

function Player:tick(delta_time)
    self:update_animation()
end

function Player:update_animation()
    local animator = self.entity:get_sprite_animator()
    if animator == nil then
        return
    end

    local velocity = self.entity:get_character_body():get_velocity()
    local moving = velocity:length_sqr() > 0.01
    local desired = moving and "run" or "idle"

    if animator:get_current_clip() ~= desired then
        animator:play(desired)
    end
end

function Player:debug_draw_tick(delta_time)
    local mouse_screen = Input.get_mouse_screen_position()
    local mouse_world = Camera.screen_to_world(mouse_screen)
    local player_pos = self.entity:get_transform():get_position()

    local direction = mouse_world - player_pos
    local distance = direction:length()
    local hit = Physics.raycast(player_pos, direction, distance)

    if hit.hit then
        Debug.draw_line(player_pos, hit.point, Color.red())
        Debug.draw_circle(hit.point, 0.1, Color.red())
    else
        Debug.draw_line(player_pos, mouse_world, Color.green())
    end
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
