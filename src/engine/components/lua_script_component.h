#pragma once

#include <string>

#include <sol/sol.hpp>

#include "component.h"

namespace hob {
    class LuaScriptComponent : public Component {
        std::string m_script_name;
        sol::table m_instance;

    public:
        explicit LuaScriptComponent(Entity& entity);

        const std::string& get_script_name() const;
        void set_script_name(std::string name);

        void enter_play() override;
        void exit_play() override;
        void tick(float delta_time) override;
        void physics_tick(float fixed_delta_time) override;
        void debug_draw_tick(float delta_time) override;
        void on_collision_enter(const ColliderComponent* other_collider) override;
        void on_collision_exit(const ColliderComponent* other_collider) override;
        void on_trigger_enter(const ColliderComponent* other_collider) override;
        void on_trigger_exit(const ColliderComponent* other_collider) override;
    };
}
