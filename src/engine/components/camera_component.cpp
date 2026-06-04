#include "camera_component.h"

#include <format>

#include "engine/core/engine.h"
#include "engine/core/systems/renderer/renderer.h"
#include "engine/entity/entity.h"
#include "transform_component.h"

namespace hob {
    CameraComponent::CameraComponent(Entity& entity)
        : Component(entity) {
    }

    void CameraComponent::enter_play() {
        get_engine().set_active_camera(this);
    }

    void CameraComponent::exit_play() {
        get_engine().clear_active_camera(this);
    }

    std::string CameraComponent::to_string() const {
        return std::format("CameraComponent(entity_id = {})", get_entity().get_id());
    }

    float CameraComponent::get_screen_pixels_per_meter() const {
        return m_screen_pixels_per_meter;
    }

    void CameraComponent::set_screen_pixels_per_meter(float value) {
        m_screen_pixels_per_meter = value;
        if (!m_base_captured) {
            m_base_screen_pixels_per_meter = value;
            m_base_captured = true;
        }
    }

    float CameraComponent::get_zoom() const {
        return m_screen_pixels_per_meter / m_base_screen_pixels_per_meter;
    }

    void CameraComponent::set_zoom(float multiplier) {
        m_screen_pixels_per_meter = m_base_screen_pixels_per_meter * multiplier;
    }

    Vector2 CameraComponent::world_to_screen(const Vector2& world_position) const {
        TransformComponent* transform = get_entity().get_transform();
        Vector2 camera_position = transform->get_position();
        Vector2 screen_position = world_to_screen(world_position, camera_position);

        return screen_position;
    }

    Vector2 CameraComponent::world_to_screen(const Vector2& world_position, const Vector2& camera_position) const {
        const Renderer& renderer = get_engine().get_renderer();

        float half_width = renderer.get_logical_width_f() * 0.5f;
        float half_height = renderer.get_logical_height_f() * 0.5f;

        // 1) Calculate world delta relative to camera
        Vector2 delta_meters = world_position - camera_position;

        // 2) Convert meters to pixels using this camera's scale
        Vector2 delta_pixels = delta_meters * m_screen_pixels_per_meter;

        // 3) Flip Y (because screen positive Y goes down)
        delta_pixels.y = -delta_pixels.y;

        // 4) Calculate screen position
        Vector2 screen_position = Vector2(delta_pixels.x + half_width, delta_pixels.y + half_height);

        return screen_position;
    }

    Vector2 CameraComponent::screen_to_world(const Vector2& screen_position) const {
        TransformComponent* transform = get_entity().get_transform();
        Vector2 camera_position = transform->get_position();
        Vector2 world_position = screen_to_world(screen_position, camera_position);

        return world_position;
    }

    Vector2 CameraComponent::screen_to_world(const Vector2& screen_position, const Vector2& camera_position) const {
        const Renderer& renderer = get_engine().get_renderer();

        float half_width = renderer.get_logical_width_f() * 0.5f;
        float half_height = renderer.get_logical_height_f() * 0.5f;

        // 1) Move origin from top-left to screen center
        Vector2 delta_pixels = Vector2(screen_position.x - half_width, screen_position.y - half_height);

        // 2) Undo Y flip (because world positive Y goes up)
        delta_pixels.y = -delta_pixels.y;

        // 3) Convert pixels to meters using this camera's scale
        Vector2 delta_meters = delta_pixels / m_screen_pixels_per_meter;

        // 4) Calculate world position
        Vector2 world_position = camera_position + delta_meters;

        return world_position;
    }
}
