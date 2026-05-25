#include "lua_script_component.h"

#include <format>

#include "engine/components/physics/collider_component.h"
#include "engine/core/app.h"
#include "engine/core/logging.h"
#include "engine/core/lua_script_system.h"
#include "engine/entity/entity.h"

namespace hob {
    LuaScriptComponent::LuaScriptComponent(Entity& entity)
        : Component(entity) {
    }

    const std::string& LuaScriptComponent::get_class_name() const {
        return m_class_name;
    }

    void LuaScriptComponent::set_class_name(std::string name) {
        m_class_name = std::move(name);
    }

    void LuaScriptComponent::enter_play() {
        init_lua_instance();

        call_hook("enter_play");
    }

    void LuaScriptComponent::exit_play() {
        call_hook("exit_play");
    }

    void LuaScriptComponent::tick(float delta_time) {
        call_hook("tick", delta_time);
    }

    void LuaScriptComponent::physics_tick(float fixed_delta_time) {
        call_hook("physics_tick", fixed_delta_time);
    }

    void LuaScriptComponent::debug_draw_tick(float delta_time) {
        call_hook("debug_draw_tick", delta_time);
    }

    void LuaScriptComponent::on_collision_enter(const ColliderComponent* other_collider) {
        call_hook("on_collision_enter", other_collider);
    }

    void LuaScriptComponent::on_collision_exit(const ColliderComponent* other_collider) {
        call_hook("on_collision_exit", other_collider);
    }

    void LuaScriptComponent::on_trigger_enter(const ColliderComponent* other_collider) {
        call_hook("on_trigger_enter", other_collider);
    }

    void LuaScriptComponent::on_trigger_exit(const ColliderComponent* other_collider) {
        call_hook("on_trigger_exit", other_collider);
    }

    std::string LuaScriptComponent::to_string() const {
        return std::format("LuaScriptComponent(entity_id = {}, class = {})",
                           get_entity().get_id(),
                           get_class_name());
    }

    void LuaScriptComponent::init_lua_instance() {
        if (m_class_name.empty()) {
            debug::log_error("LuaScriptComponent has no class name");
            return;
        }

        sol::state& lua = get_app().get_lua_script_system().get_lua();

        sol::object registry_obj = lua["__component_registry"];
        if (!registry_obj.is<sol::table>()) {
            debug::log_error("__component_registry is missing — engine bootstrap did not run");
            return;
        }

        sol::table registry = registry_obj;
        sol::object class_obj = registry[m_class_name];
        if (!class_obj.is<sol::table>()) {
            debug::log_error("DefineComponent '{}' is not registered", m_class_name);
            return;
        }

        sol::table class_table = class_obj;
        sol::object new_fn_obj = class_table["new"];
        if (!new_fn_obj.is<sol::protected_function>()) {
            debug::log_error("'{}' does not have a 'new' function", m_class_name);
            return;
        }

        sol::protected_function new_fn = new_fn_obj;
        sol::protected_function_result inst_result = new_fn();
        if (!inst_result.valid()) {
            sol::error err = inst_result;
            debug::log_error("Failed to instantiate '{}': {}", m_class_name, err.what());
            return;
        }

        sol::object inst_obj = inst_result;
        if (!inst_obj.is<sol::table>()) {
            debug::log_error("'{}'.new() did not return a table", m_class_name);
            return;
        }

        m_lua_instance = inst_obj;
        m_lua_instance["entity"] = &get_entity();
    }
}
