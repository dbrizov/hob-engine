#ifndef HOB_ENGINE_CAPSULECOLLIDERCOMPONENT_H
#define HOB_ENGINE_CAPSULECOLLIDERCOMPONENT_H
#include <box2d/id.h>

#include "ColliderComponent.h"
#include "engine/math/Capsule.h"


class CapsuleColliderComponent : public ColliderComponent {
    // Local capsule
    Capsule m_capsule = Capsule(Vector2::down() * 0.5f, Vector2::up() * 0.5f, 0.5f);

public:
    explicit CapsuleColliderComponent(Entity& entity);

    Capsule get_capsule() const;
    void set_capsule(const Capsule& capsule);

protected:
    virtual b2ShapeId create_shape(const b2ShapeDef& shape_def) override;
    virtual void debug_draw_shape(const Color& color) const override;
};


#endif //HOB_ENGINE_CAPSULECOLLIDERCOMPONENT_H
