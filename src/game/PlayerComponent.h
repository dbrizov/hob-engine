#ifndef HOB_ENGINE_PLAYERCOMPONENT_H
#define HOB_ENGINE_PLAYERCOMPONENT_H
#include "engine/components/Component.h"
#include "engine/components/InputComponent.h"
#include "engine/math/Vector2.h"


namespace game {
    class PlayerComponent : public hob::Component {
        float m_speed = 7.0f;
        float m_camera_follow_speed = 10.0f;
        hob::Vector2 m_movement_input;
        hob::BindingId m_x_axis_id = hob::INVALID_BINDING_ID;
        hob::BindingId m_y_axis_id = hob::INVALID_BINDING_ID;
        hob::BindingId m_slow_motion_action_id = hob::INVALID_BINDING_ID;

    public:
        explicit PlayerComponent(hob::Entity& entity);

        virtual void enter_play() override;
        virtual void exit_play() override;
        virtual void physics_tick(float fixed_delta_time) override;
        virtual void on_collision_enter(const hob::ColliderComponent* other_collider) override;
        virtual void on_collision_exit(const hob::ColliderComponent* other_collider) override;
        virtual void on_trigger_enter(const hob::ColliderComponent* other_collider) override;
        virtual void on_trigger_exit(const hob::ColliderComponent* other_collider) override;

    private:
        void update_camera_position(const hob::Vector2& target_position, float fixed_delta_time);

        void set_movement_input_x(float x_axis);
        void set_movement_input_y(float y_axis);
        void toggle_slow_motion();
    };
}


#endif //HOB_ENGINE_PLAYERCOMPONENT_H
