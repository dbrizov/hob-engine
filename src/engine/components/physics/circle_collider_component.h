#pragma once

#include <box2d/id.h>

#include "collider_component.h"
#include "engine/math/circle.h"

namespace hob {
    class CircleColliderComponent : public ColliderComponent {
        // Local circle
        Circle m_circle = Circle(Vector2::zero(), 0.5f);

    public:
        explicit CircleColliderComponent(Entity& entity);

        std::string to_string() const override;

        Circle get_circle() const;
        void set_circle(const Circle& circle);

        Circle get_scaled_circle() const;

    protected:
        b2ShapeId create_geometry(const b2ShapeDef& shape_def, const Vector2& scale) override;
        void update_geometry(const Vector2& scale) override;
        void debug_draw_shape(const Color& color, const Vector2& scale) const override;

    private:
        static Circle scale_circle(const Circle& local, const Vector2& scale);
    };
}
