#include "PlayerComponent.h"

#include "engine/components/TransformComponent.h"
#include "engine/entity/Entity.h"

void PlayerComponent::enter_play() {
    InputComponent* input_component = get_entity()->get_component<InputComponent>();
    m_x_axis_id = input_component->bind_axis("horizontal", [this](float axis) {
        set_movement_input_x(axis);
    });

    m_y_axis_id = input_component->bind_axis("vertical", [this](float axis) {
        set_movement_input_y(axis);
    });
}

void PlayerComponent::exit_play() {
    InputComponent* input_component = get_entity()->get_component<InputComponent>();
    input_component->unbind_axis("horizontal", m_x_axis_id);
    input_component->unbind_axis("vertical", m_y_axis_id);
}

void PlayerComponent::tick(float delta_time) {
    TransformComponent* transform = get_entity()->get_component<TransformComponent>();

    Vector2 movement_input = m_movement_input;
    if (movement_input.length_sqr() > 1.0f) {
        movement_input = movement_input.normalized();
    }

    Vector2 pos_delta_x = Vector2::right() * movement_input.x;
    Vector2 pos_delta_y = Vector2::up() * movement_input.y;
    Vector2 pos_delta = (pos_delta_x + pos_delta_y) * m_speed * delta_time;
    Vector2 new_pos = transform->get_position() + pos_delta;
    transform->set_position(new_pos);
}

void PlayerComponent::set_movement_input_x(float x_axis) {
    m_movement_input.x = x_axis;
}

void PlayerComponent::set_movement_input_y(float y_axis) {
    m_movement_input.y = y_axis;
}
