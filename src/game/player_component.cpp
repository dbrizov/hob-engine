#include "player_component.h"

#include <cmath>

#include "engine/components/camera_component.h"
#include "engine/components/physics/character_body_component.h"
#include "engine/components/physics/collider_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/app.h"
#include "engine/core/logging.h"
#include "engine/entity/entity.h"

namespace game {
    PlayerComponent::PlayerComponent(hob::Entity& entity)
        : Component(entity) {
    }

    void PlayerComponent::enter_play() {
        hob::InputComponent* input_component = get_entity().get_component<hob::InputComponent>();
        m_x_axis_id = input_component->bind_axis("horizontal", [this](float axis) {
            set_movement_input_x(axis);
        });

        m_y_axis_id = input_component->bind_axis("vertical", [this](float axis) {
            set_movement_input_y(axis);
        });

        m_slow_motion_action_id = input_component->bind_action("slow_motion", hob::InputEventType::Pressed, [this]() {
            toggle_slow_motion();
        });
    }

    void PlayerComponent::exit_play() {
        hob::InputComponent* input_component = get_entity().get_component<hob::InputComponent>();
        input_component->unbind_axis("horizontal", m_x_axis_id);
        input_component->unbind_axis("vertical", m_y_axis_id);
        input_component->unbind_action("slow_motion", m_slow_motion_action_id);
    }

    void PlayerComponent::physics_tick(float fixed_delta_time) {
        hob::Vector2 movement_input = m_movement_input;
        if (movement_input.length_sqr() > 1.0f) {
            movement_input = movement_input.normalized();
        }

        hob::Vector2 velocity = movement_input * m_speed;
        hob::CharacterBodyComponent* character_body = get_entity().get_component<hob::CharacterBodyComponent>();
        character_body->move_and_slide(velocity, fixed_delta_time);

        // The position and rotation of the player are controlled by the CharacterBody
        // which updates the transform's position and rotation at the end of each physics tick.
        // This means that the camera's position and the player's rotation are not immediately updated.
        hob::Vector2 position = get_entity().get_transform()->get_position();
        update_camera_position(position, fixed_delta_time);
        update_rotation(fixed_delta_time);
    }

    void PlayerComponent::on_collision_enter(const hob::ColliderComponent* other_collider) {
        hob::debug::log("collision_enter: {}", other_collider->get_entity().get_id());
    }

    void PlayerComponent::on_collision_exit(const hob::ColliderComponent* other_collider) {
        hob::debug::log("collision_exit: {}", other_collider->get_entity().get_id());
    }

    void PlayerComponent::on_trigger_enter(const hob::ColliderComponent* other_collider) {
        hob::debug::log("trigger_enter: {}", other_collider->get_entity().get_id());
    }

    void PlayerComponent::on_trigger_exit(const hob::ColliderComponent* other_collider) {
        hob::debug::log("trigger_exit: {}", other_collider->get_entity().get_id());
    }

    void PlayerComponent::update_camera_position(const hob::Vector2& target_position, float delta_time) {
        hob::Entity* camera_entity = get_app().get_entity_spawner().get_camera_entity();
        hob::TransformComponent* camera_transform = camera_entity->get_transform();

        hob::Vector2 new_position = hob::Vector2::lerp(
            camera_transform->get_position(), target_position, delta_time * m_camera_follow_speed);

        camera_transform->set_position(new_position);
    }

    void PlayerComponent::update_rotation(float delta_time) {
        const hob::Entity* camera_entity = get_app().get_entity_spawner().get_camera_entity();
        const hob::CameraComponent* camera_comp = camera_entity->get_component<hob::CameraComponent>();
        hob::Vector2 mouse_screen_pos = get_app().get_input().get_mouse_screen_position();
        hob::Vector2 mouse_world_pos = camera_comp->screen_to_world(mouse_screen_pos);

        hob::CharacterBodyComponent* character_body = get_entity().get_component<hob::CharacterBodyComponent>();
        hob::Vector2 player_pos = character_body->get_position();

        hob::Vector2 direction = mouse_world_pos - player_pos;

        float radians = std::atan2(direction.y, direction.x);
        character_body->set_rotation(radians);
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
}
