DefineComponent.ContactLogger = {}

function ContactLogger:on_collision_enter(other)
    log("collision_enter: " .. other:get_entity():get_id())
end

function ContactLogger:on_collision_exit(other)
    log("collision_exit: " .. other:get_entity():get_id())
end

function ContactLogger:on_trigger_enter(other)
    log("trigger_enter: " .. other:get_entity():get_id())
end

function ContactLogger:on_trigger_exit(other)
    log("trigger_exit: " .. other:get_entity():get_id())
end
