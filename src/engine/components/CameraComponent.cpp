#include "CameraComponent.h"

#include "TransformComponent.h"
#include "engine/core/App.h"
#include "engine/entity/Entity.h"

void CameraComponent::init(uint32_t logical_resolution_width, uint32_t logical_resolution_height) {
    m_logical_resolution_width = logical_resolution_width;
    m_logical_resolution_height = logical_resolution_height;
}

uint32_t CameraComponent::get_logical_resolution_width() const {
    return m_logical_resolution_width;
}

uint32_t CameraComponent::get_logical_resolution_height() const {
    return m_logical_resolution_height;
}

Vector2 CameraComponent::world_to_screen(const Vector2& world_position) const {
    TransformComponent* transform = get_entity()->get_transform();
    Vector2 camera_position = transform->get_position();
    Vector2 screen_position = world_to_screen(world_position, camera_position);

    return screen_position;
}

Vector2 CameraComponent::world_to_screen(const Vector2& world_position, const Vector2& camera_position) const {
    float half_width = static_cast<float>(m_logical_resolution_width) / 2.0f;
    float half_height = static_cast<float>(m_logical_resolution_height) / 2.0f;

    Vector2 screen_position = Vector2(
        world_position.x - camera_position.x + half_width,
        world_position.y - camera_position.y + half_height);

    return screen_position;
}
