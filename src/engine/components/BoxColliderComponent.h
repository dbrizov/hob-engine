#ifndef HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
#define HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
#include <box2d/id.h>

#include "Component.h"
#include "engine/math/AABB.h"


class BoxColliderComponent : public Component {
    b2ShapeId m_shape_id = b2_nullShapeId;
    AABB m_aabb = AABB(Vector2(0.0f, 0.0f), Vector2(0.5f, 0.5f)); /// Local AABB
    float m_density = 1.0f; /// In kg/m^2. A body's <code>mass = density * area</code>
    float m_friction = 0.6f; /// In range [0, 1]
    float m_bounciness = 0.0f; /// In range [0, 1]
    uint64_t m_collision_layer = 1u; /// The collision layer of this collider
    uint64_t m_collision_mask = ~0u; /// What this collider collides with
    bool m_is_trigger = false;

public:
    explicit BoxColliderComponent(Entity& entity);

    virtual void enter_play() override;
    virtual void exit_play() override;
    virtual void render_tick(float delta_time, RenderQueue& render_queue) override;

    AABB get_aabb() const;
    void set_aabb(const AABB& aabb);

    float get_density() const;
    void set_density(float density);

    float get_friction() const;
    void set_friction(float friction);

    float get_bounciness() const;
    void set_bounciness(float bounciness);

    uint64_t get_collision_layer() const;
    void set_collision_layer(uint64_t collision_layer);

    uint64_t get_collision_mask() const;
    void set_collision_mask(uint64_t collision_mask);

    bool is_trigger() const;
    void set_trigger(bool trigger);
};


#endif //HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
