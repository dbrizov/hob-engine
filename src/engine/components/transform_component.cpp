#include "transform_component.h"

#include <cmath>
#include <format>

#include "engine/components/physics/collider_component.h"
#include "engine/entity/entity.h"

namespace hob {
    TransformComponent::TransformComponent(Entity& entity)
        : Component(entity) {
        rebuild_local_matrix();
    }

    std::string TransformComponent::to_string() const {
        return std::format("TransformComponent(entity_id = {})", get_entity().get_id());
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
        if (m_scale == scale) {
            return;
        }

        m_scale = scale;
        rebuild_local_matrix();

        // Scale feeds into the physics shape geometry; have each collider re-sync.
        for (ColliderComponent* collider : get_entity().get_components<ColliderComponent>()) {
            collider->on_changed();
        }
    }

    bool TransformComponent::get_interpolate_physics() const {
        return m_interpolate_physics;
    }

    void TransformComponent::set_interpolate_physics(bool value) {
        if (m_interpolate_physics == value) {
            return;
        }

        m_interpolate_physics = value;
        if (value) {
            // Bring prev back in sync so the first render after re-enabling doesn't lerp from a stale matrix.
            m_prev_local_matrix = m_local_matrix;
        }
    }

    const Matrix2x3& TransformComponent::get_local_matrix() const {
        return m_local_matrix;
    }

    const Matrix2x3& TransformComponent::get_prev_local_matrix() const {
        return m_prev_local_matrix;
    }

    void TransformComponent::rebuild_local_matrix() {
        const float cos = std::cos(m_rotation);
        const float sin = std::sin(m_rotation);

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
