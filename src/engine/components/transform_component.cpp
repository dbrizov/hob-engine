#include "transform_component.h"

#include "engine/entity/entity.h"
#include "engine/math/mathf.h"

namespace hob {
    TransformComponent::TransformComponent(Entity& entity)
        : Component(entity) {
    }

    Vector2 TransformComponent::get_position() const {
        return m_position;
    }

    void TransformComponent::set_position(const Vector2& position) {
        m_position = position;

        if (!get_entity().is_in_play()) {
            // The entity isn't spawned yet. Match positions to prevent initial Physics interpolation
            m_prev_physics_position = m_position;
        }
    }

    float TransformComponent::get_rotation() const {
        return m_rotation;
    }

    void TransformComponent::set_rotation(float rotation_degrees) {
        m_rotation = math::normalize_angle(rotation_degrees);

        if (!get_entity().is_in_play()) {
            // The entity isn't spawned yet. Match rotations to prevent initial Physics interpolation
            m_prev_physics_rotation = m_rotation;
        }
    }

    Vector2 TransformComponent::get_scale() const {
        return m_scale;
    }

    void TransformComponent::set_scale(const Vector2& scale) {
        m_scale = scale;
    }

    Vector2 TransformComponent::get_prev_physics_position() const {
        return m_prev_physics_position;
    }

    void TransformComponent::set_prev_physics_position(const Vector2& position) {
        m_prev_physics_position = position;
    }

    float TransformComponent::get_prev_physics_rotation() const {
        return m_prev_physics_rotation;
    }

    void TransformComponent::set_prev_physics_rotation(float rotation_degrees) {
        m_prev_physics_rotation = rotation_degrees;
    }
}
