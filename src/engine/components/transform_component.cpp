#include "transform_component.h"

#include <cmath>

#include "engine/entity/entity.h"

namespace hob {
    TransformComponent::TransformComponent(Entity& entity)
        : Component(entity) {
        rebuild_local_matrix();
    }

    Vector2 TransformComponent::get_position() const {
        return m_position;
    }

    void TransformComponent::set_position(const Vector2& position) {
        m_position = position;
        m_local_matrix.origin = m_position;

        if (!get_entity().is_in_play()) {
            // The entity isn't spawned yet. Match translation to prevent initial Physics interpolation
            m_prev_local_matrix.origin = m_local_matrix.origin;
        }
    }

    float TransformComponent::get_rotation() const {
        return m_rotation;
    }

    void TransformComponent::set_rotation(float radians) {
        m_rotation = radians;
        rebuild_local_matrix();
    }

    Vector2 TransformComponent::get_scale() const {
        return m_scale;
    }

    void TransformComponent::set_scale(const Vector2& scale) {
        m_scale = scale;
        rebuild_local_matrix();
    }

    const Matrix2x3& TransformComponent::get_local_matrix() const {
        return m_local_matrix;
    }

    const Matrix2x3& TransformComponent::get_prev_local_matrix() const {
        return m_prev_local_matrix;
    }

    void TransformComponent::rebuild_local_matrix() {
        float cos = std::cos(m_rotation);
        float sin = std::sin(m_rotation);

        m_local_matrix.x = Vector2(cos * m_scale.x, sin * m_scale.x);
        m_local_matrix.y = Vector2(-sin * m_scale.y, cos * m_scale.y);
        m_local_matrix.origin = m_position;

        if (!get_entity().is_in_play()) {
            // The entity isn't spawned yet. Match local matrices to prevent initial Physics interpolation
            m_prev_local_matrix = m_local_matrix;
        }
    }

    void TransformComponent::set_prev_local_matrix(const Matrix2x3& prev_local_matrix) {
        m_prev_local_matrix = prev_local_matrix;
    }
}
