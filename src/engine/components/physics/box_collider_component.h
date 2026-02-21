#ifndef HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
#define HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
#include <box2d/id.h>

#include "collider_component.h"
#include "engine/math/aabb.h"

namespace hob {
    class BoxColliderComponent : public ColliderComponent {
        // Local AABB
        AABB m_aabb = AABB(Vector2(0.0f, 0.0f), Vector2(0.5f, 0.5f));

    public:
        explicit BoxColliderComponent(Entity& entity);

        AABB get_aabb() const;
        void set_aabb(const AABB& aabb);

    protected:
        virtual b2ShapeId create_shape(const b2ShapeDef& shape_def) override;
        virtual void debug_draw_shape(const Color& color) const override;
    };
}

#endif //HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
