DefineComponent.ContactLogger = {}
---@class ContactLogger : LuaComponent
local ContactLogger = ContactLogger

function ContactLogger:init()
    log("ContactLogger:init()")
end

function ContactLogger:enter_play()
    log("ContactLogger:enter_play()")
end

function ContactLogger:exit_play()
    log("ContactLogger:exit_play()")
end

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
