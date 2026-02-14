#ifndef HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
#define HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
#include <box2d/id.h>

#include "Component.h"
#include "engine/math/Vector2.h"


class BoxColliderComponent : public Component {
    b2ShapeId m_shape_id = b2_nullShapeId;

    // Local box
    float m_half_width = 0.5f;
    float m_half_height = 0.5f;
    Vector2 m_center = Vector2(0.0f, 0.0f);

    // Physics properties
    float m_density = 1.0f; // Used for dynamic bodies
    float m_friction = 0.6f;
    float m_restitution = 0.0f;

    // Collision filtering
    bool m_is_trigger = false;
    uint16_t m_category_bits = 0x0001;
    uint16_t m_mask_bits = 0xFFFF;

public:
    explicit BoxColliderComponent(Entity& entity);

    void enter_play() override;
    void exit_play() override;
    void render_tick(float delta_time, RenderQueue& render_queue) override;
};


#endif //HOB_ENGINE_BOXCOLLIDERCOMPONENT_H
