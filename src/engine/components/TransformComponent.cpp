#include "TransformComponent.h"

ComponentPriority TransformComponent::get_priority() const {
    return ComponentPriority::TRANSFORM;
}

Vector2 TransformComponent::get_position() const {
    return m_position;
}

void TransformComponent::set_position(const Vector2& position) {
    m_prev_position = m_position;
    m_position = position;
}

Vector2 TransformComponent::get_prev_position() const {
    return m_prev_position;
}

Vector2 TransformComponent::get_scale() const {
    return m_scale;
}

void TransformComponent::set_scale(const Vector2& scale) {
    m_scale = scale;
}
