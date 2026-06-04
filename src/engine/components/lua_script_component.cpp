#include "lua_script_component.h"
#include "lua_script_component_impl.h"

#include <format>
#include <utility>

#include "engine/components/physics/collider_component.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/systems/scripting/lua_script_system.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_ref.h"

namespace hob {
    namespace {
        template<typename... Args>
        void call_hook(LuaScriptComponentImpl& impl, const char* method, Args&&... args) {
            sol::table& inst = impl.lua_instance;
            if (!inst.valid()) {
                return;
            }

            sol::object fn = inst[method];
            if (!fn.is<sol::protected_function>()) {
                return;
            }

            sol::protected_function pfn = fn;
            sol::protected_function_result result = pfn(inst, std::forward<Args>(args)...);
            if (!result.valid()) {
                sol::error err = result;
                debug::log_error("Lua error in {}: {}", method, err.what());
            }
        }
    }

    LuaScriptComponent::LuaScriptComponent(Entity& entity, std::string class_name)
        : Component(entity)
        , m_class_name(std::move(class_name))
        , m_impl(std::make_unique<LuaScriptComponentImpl>())
        , m_priority(component_priority::CP_DEFAULT) {
    }

    LuaScriptComponent::~LuaScriptComponent() = default;

    const std::string& LuaScriptComponent::get_class_name() const {
        return m_class_name;
    }

    LuaScriptComponentImpl& LuaScriptComponent::impl() {
        return *m_impl;
    }

    const LuaScriptComponentImpl& LuaScriptComponent::impl() const {
        return *m_impl;
    }

    int LuaScriptComponent::get_priority() const {
        return m_priority;
    }

    void LuaScriptComponent::init() {
        if (m_class_name.empty()) {
            debug::log_error("LuaScriptComponent has no class name");
            return;
        }

        sol::state& lua = get_engine().get_lua_script_system().get_lua();

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

        sol::optional<int> priority = class_table["priority"];
        m_priority = priority.value_or(component_priority::CP_DEFAULT);

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

        m_impl->lua_instance = inst_obj;
        m_impl->lua_instance["entity"] = EntityRef(get_entity().get_id(), get_engine().get_entity_spawner());
        m_impl->lua_instance["class_name"] = m_class_name;

        call_hook(*m_impl, "init");
    }

    void LuaScriptComponent::enter_play() {
        call_hook(*m_impl, "enter_play");
    }

    void LuaScriptComponent::exit_play() {
        call_hook(*m_impl, "exit_play");
    }

    void LuaScriptComponent::tick(float delta_time) {
        call_hook(*m_impl, "tick", delta_time);
    }

    void LuaScriptComponent::physics_tick(float fixed_delta_time) {
        call_hook(*m_impl, "physics_tick", fixed_delta_time);
    }

    void LuaScriptComponent::debug_draw_tick(float delta_time) {
        call_hook(*m_impl, "debug_draw_tick", delta_time);
    }

    void LuaScriptComponent::on_collision_enter(const ColliderComponent* other_collider) {
        call_hook(*m_impl, "on_collision_enter", other_collider);
    }

    void LuaScriptComponent::on_collision_exit(const ColliderComponent* other_collider) {
        call_hook(*m_impl, "on_collision_exit", other_collider);
    }

    void LuaScriptComponent::on_trigger_enter(const ColliderComponent* other_collider) {
        call_hook(*m_impl, "on_trigger_enter", other_collider);
    }

    void LuaScriptComponent::on_trigger_exit(const ColliderComponent* other_collider) {
        call_hook(*m_impl, "on_trigger_exit", other_collider);
    }

    std::string LuaScriptComponent::to_string() const {
        return std::format("LuaScriptComponent(entity_id = {}, class = {})",
                           get_entity().get_id(),
                           get_class_name());
    }
}
