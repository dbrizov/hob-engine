#pragma once

#include <box2d/id.h>

#include "collider_component.h"
#include "engine/math/capsule.h"

namespace hob {
    class CapsuleColliderComponent : public ColliderComponent {
        // Local capsule
        Capsule m_capsule = Capsule(Vector2::down() * 0.5f, Vector2::up() * 0.5f, 0.5f);

    public:
        explicit CapsuleColliderComponent(Entity& entity);

        std::string to_string() const override;

        Capsule get_capsule() const;
        void set_capsule(const Capsule& capsule);

        Capsule get_scaled_capsule() const;

    protected:
        b2ShapeId create_shape(const b2ShapeDef& shape_def, const Vector2& scale) override;
        void rebuild_shape(const Vector2& scale) override;
        void debug_draw_shape(const Color& color, const Vector2& scale) const override;

    private:
        static Capsule scale_capsule(const Capsule& local, const Vector2& scale);
    };
}
