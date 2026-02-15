#include "CameraComponent.h"

#include "TransformComponent.h"
#include "engine/core/App.h"
#include "engine/entity/Entity.h"

void CameraComponent::init(uint32_t logical_resolution_width, uint32_t logical_resolution_height) {
    m_logical_resolution_width = logical_resolution_width;
    m_logical_resolution_height = logical_resolution_height;
}

CameraComponent::CameraComponent(Entity& entity)
    : Component(entity) {
}

uint32_t CameraComponent::get_logical_resolution_width() const {
    return m_logical_resolution_width;
}

uint32_t CameraComponent::get_logical_resolution_height() const {
    return m_logical_resolution_height;
}

Vector2 CameraComponent::world_to_screen(const Vector2& world_position) const {
    TransformComponent* transform = get_entity().get_transform();
    Vector2 camera_position = transform->get_position();
    Vector2 screen_position = world_to_screen(world_position, camera_position);

    return screen_position;
}

Vector2 CameraComponent::world_to_screen(const Vector2& world_position, const Vector2& camera_position) const {
    float half_width = static_cast<float>(m_logical_resolution_width) * 0.5f;
    float half_height = static_cast<float>(m_logical_resolution_height) * 0.5f;

    // 1) Calculate world delta relative to camera
    Vector2 delta_meters = world_position - camera_position;

    // 2) Convert to pixels
    Vector2 delta_pixels = delta_meters * get_app().get_config().graphics_config.pixels_per_meter;

    // 3) Flip Y (because screen Y goes down)
    delta_pixels.y = -delta_pixels.y;

    Vector2 screen_position = Vector2(delta_pixels.x + half_width, delta_pixels.y + half_height);

    return screen_position;
}
