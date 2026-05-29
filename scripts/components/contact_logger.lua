DefineComponent.ContactLogger = {}
---@class ContactLogger : LuaComponent
local ContactLogger = ContactLogger

function ContactLogger:on_collision_enter(other)
    Debug.log("collision_enter: " .. other:get_entity():get_id())
end

function ContactLogger:on_collision_exit(other)
    Debug.log("collision_exit: " .. other:get_entity():get_id())
end

function ContactLogger:on_trigger_enter(other)
    Debug.log("trigger_enter: " .. other:get_entity():get_id())
end

function ContactLogger:on_trigger_exit(other)
    Debug.log("trigger_exit: " .. other:get_entity():get_id())
end
