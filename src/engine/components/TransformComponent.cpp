#include "TransformComponent.h"

#include "engine/entity/Entity.h"

TransformComponent::TransformComponent(Entity& entity)
    : Component(entity) {
}

int TransformComponent::get_priority() const {
    return component_priority::CP_TRANSFORM;
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

float TransformComponent::get_rotation() const {
    return m_rotation;
}

void TransformComponent::set_rotation(float rotation) {
    m_rotation = rotation;
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
