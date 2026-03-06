#pragma once

#include "component.h"
#include "engine/math/matrix2x3.h"
#include "engine/math/vector2.h"

namespace hob {
    class TransformComponent : public Component {
        Vector2 m_position;
        float m_rotation = 0.0f; // in degrees
        Vector2 m_scale = Vector2(1.0f, 1.0f);

        Matrix2x3 m_local_matrix;
        Matrix2x3 m_prev_local_matrix; // Used for Physics interpolation

        // Physics is a friend class of TransformComponent so that
        // the rendering can take advantage of Physics interpolation when enabled.
        friend class Physics;

    public:
        explicit TransformComponent(Entity& entity);

        Vector2 get_position() const;
        void set_position(const Vector2& position);

        float get_rotation() const;
        void set_rotation(float rotation_degrees);

        Vector2 get_scale() const;
        void set_scale(const Vector2& scale);

        const Matrix2x3& get_local_matrix() const;
        const Matrix2x3& get_prev_local_matrix() const;

    private:
        void rebuild_local_matrix();
        void set_prev_local_matrix(const Matrix2x3& prev_local_matrix);
    };
}
