#include "camera_component.h"

#include "transform_component.h"
#include "engine/core/app.h"
#include "engine/entity/entity.h"

namespace hob {
    CameraComponent::CameraComponent(Entity& entity)
        : Component(entity) {
    }

    Vector2 CameraComponent::world_to_screen(const Vector2& world_position) const {
        TransformComponent* transform = get_entity().get_transform();
        Vector2 camera_position = transform->get_position();
        Vector2 screen_position = world_to_screen(world_position, camera_position);

        return screen_position;
    }

    Vector2 CameraComponent::world_to_screen(const Vector2& world_position, const Vector2& camera_position) const {
        const GraphicsConfig& graphics_config = get_app().get_config().graphics_config;

        float half_width = static_cast<float>(graphics_config.logical_resolution_width) * 0.5f;
        float half_height = static_cast<float>(graphics_config.logical_resolution_height) * 0.5f;

        // 1) Calculate world delta relative to camera
        Vector2 delta_meters = world_position - camera_position;

        // 2) Convert meters to pixels
        float pixels_per_meter_f = static_cast<float>(graphics_config.pixels_per_meter);
        Vector2 delta_pixels = delta_meters * pixels_per_meter_f;

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
        const GraphicsConfig& graphics_config = get_app().get_config().graphics_config;

        float half_width = static_cast<float>(graphics_config.logical_resolution_width) * 0.5f;
        float half_height = static_cast<float>(graphics_config.logical_resolution_height) * 0.5f;

        // 1) Move origin from top-left to screen center
        Vector2 delta_pixels = Vector2(screen_position.x - half_width, screen_position.y - half_height);

        // 2) Undo Y flip (because world positive Y goes up)
        delta_pixels.y = -delta_pixels.y;

        // 3) Convert pixels to meters
        float pixels_per_meter_f = static_cast<float>(graphics_config.pixels_per_meter);
        Vector2 delta_meters = delta_pixels / pixels_per_meter_f;

        // 4) Calculate world position
        Vector2 world_position = camera_position + delta_meters;

        return world_position;
    }
}
