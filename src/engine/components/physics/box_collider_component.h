#pragma once

#include <box2d/id.h>

#include "collider_component.h"
#include "engine/math/aabb.h"

namespace hob {
    class BoxColliderComponent : public ColliderComponent {
        // Local AABB
        AABB m_aabb = AABB(Vector2(0.0f, 0.0f), Vector2(0.5f, 0.5f));

    public:
        explicit BoxColliderComponent(Entity& entity);

        std::string to_string() const override;

        AABB get_aabb() const;
        void set_aabb(const AABB& aabb);

        AABB get_scaled_aabb() const;

    protected:
        virtual b2ShapeId create_shape(const b2ShapeDef& shape_def, const Vector2& scale) override;
        virtual void rebuild_shape(const Vector2& scale) override;
        virtual void debug_draw_shape(const Color& color, const Vector2& scale) const override;

    private:
        static AABB scale_aabb(const AABB& local, const Vector2& scale);
    };
}
