#include "PlayerComponent.h"

#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"
#include "engine/entity/Entity.h"

void PlayerComponent::enter_play() {
    InputComponent* input_component = get_entity()->get_component<InputComponent>();
    m_x_axis_id = input_component->bind_axis("horizontal", [this](float axis) {
        set_movement_input_x(axis);
    });

    m_y_axis_id = input_component->bind_axis("vertical", [this](float axis) {
        set_movement_input_y(axis);
    });

    m_slow_motion_action_id = input_component->bind_action("slow_motion", InputEventType::PRESSED, [this]() {
        toggle_slow_motion();
    });
}

void PlayerComponent::exit_play() {
    InputComponent* input_component = get_entity()->get_component<InputComponent>();
    input_component->unbind_axis("horizontal", m_x_axis_id);
    input_component->unbind_axis("vertical", m_y_axis_id);
    input_component->unbind_action("slow_motion", m_slow_motion_action_id);
}

void PlayerComponent::physics_tick(float fixed_delta_time) {
    TransformComponent* transform = get_entity()->get_component<TransformComponent>();

    Vector2 movement_input = m_movement_input;
    if (movement_input.length_sqr() > 1.0f) {
        movement_input = movement_input.normalized();
    }

    Vector2 pos_delta_x = Vector2::right() * movement_input.x;
    Vector2 pos_delta_y = Vector2::up() * movement_input.y;
    Vector2 pos_delta = (pos_delta_x + pos_delta_y) * m_speed * fixed_delta_time;
    Vector2 new_pos = transform->get_position() + pos_delta;
    transform->set_position(new_pos);
}

void PlayerComponent::set_movement_input_x(float x_axis) {
    m_movement_input.x = x_axis;
}

void PlayerComponent::set_movement_input_y(float y_axis) {
    m_movement_input.y = y_axis;
}

void PlayerComponent::toggle_slow_motion() {
    float new_time_scale = get_app()->get_timer()->get_time_scale() < 1.0f ? 1.0f : 0.2f;
    get_app()->get_timer()->set_time_scale(new_time_scale);
}
