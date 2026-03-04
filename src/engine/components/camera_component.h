#pragma once

#include "component.h"
#include "engine/math/vector2.h"

namespace hob {
    class CameraComponent : public Component {
    public:
        explicit CameraComponent(Entity& entity);

        Vector2 world_to_screen(const Vector2& world_position) const;
        Vector2 world_to_screen(const Vector2& world_position, const Vector2& camera_position) const;
    };
}
