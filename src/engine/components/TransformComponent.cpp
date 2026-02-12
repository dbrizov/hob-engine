#include "TransformComponent.h"

#include "engine/entity/Entity.h"

TransformComponent::TransformComponent(Entity& entity)
    : Component(entity) {
}

ComponentPriority TransformComponent::get_priority() const {
    return ComponentPriority::TRANSFORM;
}

Vector2 TransformComponent::get_position() const {
    return m_position;
}

void TransformComponent::set_position(const Vector2& position) {
    m_position = position;

    if (!get_entity().is_in_play()) {
        // The entity isn't spawned yet. Match positions to prevent initial Physics interpolation
        m_prev_position = m_position;
    }
}

Vector2 TransformComponent::get_scale() const {
    return m_scale;
}

void TransformComponent::set_scale(const Vector2& scale) {
    m_scale = scale;
}

Vector2 TransformComponent::get_prev_position() const {
    return m_prev_position;
}

void TransformComponent::set_prev_position(const Vector2& prev_position) {
    m_prev_position = prev_position;
}
