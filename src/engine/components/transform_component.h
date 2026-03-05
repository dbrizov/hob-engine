#pragma once

#include "component.h"
#include "engine/math/vector2.h"

namespace hob {
    class TransformComponent : public Component {
        Vector2 m_position;
        float m_rotation = 0.0f; // in degrees
        Vector2 m_scale = Vector2(1.0f, 1.0f);

        // App and Physics are a friend classes of TransformComponent so that
        // the rendering can take advantage ot Physics interpolation when enabled.
        friend class App;
        friend class Physics;
        Vector2 m_prev_physics_position;
        float m_prev_physics_rotation = 0.0f; // in degrees

    public:
        explicit TransformComponent(Entity& entity);

        Vector2 get_position() const;
        void set_position(const Vector2& position);

        float get_rotation() const;
        void set_rotation(float rotation_degrees);

        Vector2 get_scale() const;
        void set_scale(const Vector2& scale);

    private:
        Vector2 get_prev_physics_position() const;
        void set_prev_physics_position(const Vector2& position);

        float get_prev_physics_rotation() const;
        void set_prev_physics_rotation(float rotation_degrees);
    };
}
