#pragma once

#include <string>
#include <utility>

#include <sol/sol.hpp>

#include "component.h"
#include "engine/core/logging.h"

namespace hob {
    class LuaScriptComponent : public Component {
        std::string m_class_name;
        sol::table m_lua_instance;

    public:
        explicit LuaScriptComponent(Entity& entity);

        const std::string& get_class_name() const;
        void set_class_name(std::string name);

        void enter_play() override;
        void exit_play() override;
        void tick(float delta_time) override;
        void physics_tick(float fixed_delta_time) override;
        void debug_draw_tick(float delta_time) override;
        void on_collision_enter(const ColliderComponent* other_collider) override;
        void on_collision_exit(const ColliderComponent* other_collider) override;
        void on_trigger_enter(const ColliderComponent* other_collider) override;
        void on_trigger_exit(const ColliderComponent* other_collider) override;

    private:
        void init_lua_instance();

        template<typename... Args>
        void call_hook(const char* method, Args&&... args) {
            if (!m_lua_instance.valid()) {
                return;
            }

            sol::object fn = m_lua_instance[method];
            if (!fn.is<sol::protected_function>()) {
                return;
            }

            sol::protected_function pfn = fn;
            sol::protected_function_result result = pfn(m_lua_instance, std::forward<Args>(args)...);
            if (!result.valid()) {
                sol::error err = result;
                debug::log_error("Lua error in {}: {}", method, err.what());
            }
        }
    };
}
