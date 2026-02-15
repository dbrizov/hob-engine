#ifndef HOB_ENGINE_PLAYERCOMPONENT_H
#define HOB_ENGINE_PLAYERCOMPONENT_H
#include "engine/components/Component.h"
#include "engine/components/InputComponent.h"
#include "engine/math/Vector2.h"


class PlayerComponent : public Component {
    float m_speed = 5.0f;
    float m_camera_follow_speed = 10.0f;
    Vector2 m_movement_input;
    BindingId m_x_axis_id = INVALID_BINDING_ID;
    BindingId m_y_axis_id = INVALID_BINDING_ID;
    BindingId m_slow_motion_action_id = INVALID_BINDING_ID;

public:
    explicit PlayerComponent(Entity& entity);

    virtual void enter_play() override;
    virtual void exit_play() override;
    virtual void physics_tick(float fixed_delta_time) override;
    virtual void render_tick(float delta_time, RenderQueue& render_queue) override;

private:
    void update_camera_position(const Vector2& target_position, float fixed_delta_time);

    void set_movement_input_x(float x_axis);
    void set_movement_input_y(float y_axis);
    void toggle_slow_motion();
};


#endif //HOB_ENGINE_PLAYERCOMPONENT_H
