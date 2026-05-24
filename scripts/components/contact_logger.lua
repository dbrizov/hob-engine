local ContactLogger = {}
ContactLogger.__index = ContactLogger

function ContactLogger.new()
    return setmetatable({}, ContactLogger)
end

function ContactLogger:on_collision_enter(other)
    log_error("collision_enter: " .. other:get_entity():get_id())
end

function ContactLogger:on_collision_exit(other)
    log_error("collision_exit: " .. other:get_entity():get_id())
end

function ContactLogger:on_trigger_enter(other)
    log_error("trigger_enter: " .. other:get_entity():get_id())
end

function ContactLogger:on_trigger_exit(other)
    log_error("trigger_exit: " .. other:get_entity():get_id())
end

return ContactLogger
