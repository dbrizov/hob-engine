#ifndef HOB_ENGINE_COLLIDERCOMPONENT_H
#define HOB_ENGINE_COLLIDERCOMPONENT_H
#include <box2d/id.h>

#include "Component.h"


struct Color;


class ColliderComponent : public Component {
protected:
    b2ShapeId m_shape_id = b2_nullShapeId;
    float m_density = 1.0f; // In kg/m^2. A body's mass = density * area
    float m_friction = 0.6f; // In range [0, 1]
    float m_bounciness = 0.0f; // In range [0, 1]
    uint64_t m_collision_layer = 1u; // The collision layer of this collider
    uint64_t m_collision_mask = ~0u; // What this collider collides with
    bool m_is_trigger = false;

public:
    explicit ColliderComponent(Entity& entity);

    virtual void enter_play() override;
    virtual void exit_play() override;
    virtual void debug_draw_tick(float delta_time) override;

    b2BodyId get_body_id() const;
    b2ShapeId get_shape_id() const;

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

protected:
    virtual b2ShapeId create_shape() = 0;
    virtual void debug_draw_shape(const Color& color) const = 0;
};


#endif //HOB_ENGINE_COLLIDERCOMPONENT_H
