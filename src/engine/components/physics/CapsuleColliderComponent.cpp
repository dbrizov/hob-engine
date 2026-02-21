#include "CapsuleColliderComponent.h"

#include <box2d/types.h>

#include "engine/components/TransformComponent.h"
#include "engine/core/Debug.h"
#include "engine/core/Physics.h"
#include "engine/entity/Entity.h"
#include "engine/math/Constants.h"

namespace hob {
    CapsuleColliderComponent::CapsuleColliderComponent(Entity& entity)
        : ColliderComponent(entity) {
    }

    Capsule CapsuleColliderComponent::get_capsule() const {
        return m_capsule;
    }

    void CapsuleColliderComponent::set_capsule(const Capsule& capsule) {
        m_capsule = capsule;
    }

    b2ShapeId CapsuleColliderComponent::create_shape(const b2ShapeDef& shape_def) {
        b2Capsule b2_capsule;
        b2_capsule.center1 = Physics::vec2_to_b2Vec2(m_capsule.center_a);
        b2_capsule.center2 = Physics::vec2_to_b2Vec2(m_capsule.center_b);
        b2_capsule.radius = m_capsule.radius;

        b2ShapeId shape_id = b2CreateCapsuleShape(get_body_id(), &shape_def, &b2_capsule);
        return shape_id;
    }

    void CapsuleColliderComponent::debug_draw_shape(const Color& color) const {
        const TransformComponent* transform = get_entity().get_transform();
        Vector2 position = transform->get_position();
        float rotation = transform->get_rotation_degrees();

        // Capsule's centers in world space
        Vector2 c1_world = Vector2::rotate_around(position + m_capsule.center_a, position, rotation);
        Vector2 c2_world = Vector2::rotate_around(position + m_capsule.center_b, position, rotation);

        // Capsule's axis and its perpendicular (both unit length)
        Vector2 axis = (c2_world - c1_world);
        if (axis.length_sqr() > EPSILON) {
            axis = axis.normalized();
        }
        else {
            axis = Vector2::up(); // Arbitrary if it's basically a circle
        }

        Vector2 perp(-axis.y, axis.x);

        // Capsule's side line endpoints (tangent lines)
        Vector2 p1 = c1_world + perp * m_capsule.radius;
        Vector2 p2 = c2_world + perp * m_capsule.radius;
        Vector2 p3 = c2_world - perp * m_capsule.radius;
        Vector2 p4 = c1_world - perp * m_capsule.radius;

        // Draw
        debug::draw_line(p1, p2, color);
        debug::draw_line(p3, p4, color);

        debug::draw_circle(c1_world, m_capsule.radius, color);
        debug::draw_circle(c2_world, m_capsule.radius, color);
    }
}
