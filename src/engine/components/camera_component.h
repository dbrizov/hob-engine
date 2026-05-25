#pragma once

#include "component.h"
#include "engine/math/vector2.h"

namespace hob {
    class CameraComponent : public Component {
    public:
        explicit CameraComponent(Entity& entity);

        std::string to_string() const override;

        Vector2 world_to_screen(const Vector2& world_position) const;
        Vector2 world_to_screen(const Vector2& world_position, const Vector2& camera_position) const;

        Vector2 screen_to_world(const Vector2& screen_position) const;
        Vector2 screen_to_world(const Vector2& screen_position, const Vector2& camera_position) const;
    };
}
