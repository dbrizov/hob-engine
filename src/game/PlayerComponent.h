#ifndef CPP_PLATFORMER_PLAYERCOMPONENT_H
#define CPP_PLATFORMER_PLAYERCOMPONENT_H
#include "engine/components/Component.h"
#include "engine/components/InputComponent.h"
#include "engine/math/Vector2.h"


class PlayerComponent : public Component {
    float m_speed = 150.0f;
    Vector2 m_movement_input;
    BindingId m_x_axis_id = 0;
    BindingId m_y_axis_id = 0;
    BindingId m_slow_motion_action_id = 0;

public:
    virtual void enter_play() override;
    virtual void exit_play() override;
    virtual void physics_tick(float fixed_delta_time) override;

private:
    void set_movement_input_x(float x_axis);
    void set_movement_input_y(float y_axis);
    void toggle_slow_motion();
};


#endif //CPP_PLATFORMER_PLAYERCOMPONENT_H