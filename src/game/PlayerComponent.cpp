#include "PlayerComponent.h"

#include <fmt/base.h>

#include "engine/components/CameraComponent.h"
#include "engine/components/physics/CharacterBodyComponent.h"
#include "engine/components/physics/ColliderComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"
#include "engine/entity/Entity.h"

PlayerComponent::PlayerComponent(Entity& entity)
    : Component(entity) {
}

void PlayerComponent::enter_play() {
    InputComponent* input_component = get_entity().get_component<InputComponent>();
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
    InputComponent* input_component = get_entity().get_component<InputComponent>();
    input_component->unbind_axis("horizontal", m_x_axis_id);
    input_component->unbind_axis("vertical", m_y_axis_id);
    input_component->unbind_action("slow_motion", m_slow_motion_action_id);
}

void PlayerComponent::physics_tick(float fixed_delta_time) {
    Vector2 movement_input = m_movement_input;
    if (movement_input.length_sqr() > 1.0f) {
        movement_input = movement_input.normalized();
    }

    Vector2 velocity = movement_input * m_speed;
    get_entity().get_component<CharacterBodyComponent>()->move_and_slide(velocity, fixed_delta_time);

    // TODO The camera position is not accurate, because the physics hasn't update the transform's position yet
    Vector2 position = get_entity().get_transform()->get_position();
    update_camera_position(position, fixed_delta_time);
}

void PlayerComponent::on_collision_enter(const ColliderComponent* other_collider) {
    fmt::println("collision_enter: {}", other_collider->get_entity().get_id());
}

void PlayerComponent::on_collision_exit(const ColliderComponent* other_collider) {
    fmt::println("collision_exit: {}", other_collider->get_entity().get_id());
}

void PlayerComponent::on_trigger_enter(const ColliderComponent* other_collider) {
    fmt::println("trigger_enter: {}", other_collider->get_entity().get_id());
}

void PlayerComponent::on_trigger_exit(const ColliderComponent* other_collider) {
    fmt::println("trigger_exit: {}", other_collider->get_entity().get_id());
}

void PlayerComponent::update_camera_position(const Vector2& target_position, float fixed_delta_time) {
    Entity* camera_entity = get_app().get_entity_spawner().get_camera_entity();
    TransformComponent* camera_transform = camera_entity->get_transform();

    Vector2 new_position = Vector2::lerp(
        camera_transform->get_position(), target_position, fixed_delta_time * m_camera_follow_speed);

    camera_transform->set_position(new_position);
}

void PlayerComponent::set_movement_input_x(float x_axis) {
    m_movement_input.x = x_axis;
}

void PlayerComponent::set_movement_input_y(float y_axis) {
    m_movement_input.y = y_axis;
}

void PlayerComponent::toggle_slow_motion() {
    float new_time_scale = get_app().get_timer().get_time_scale() < 1.0f ? 1.0f : 0.2f;
    get_app().get_timer().set_time_scale(new_time_scale);
}
