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

    protected:
        virtual b2ShapeId create_shape(const b2ShapeDef& shape_def) override;
        virtual void debug_draw_shape(const Color& color) const override;
    };
}
