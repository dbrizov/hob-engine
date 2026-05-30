#include "circle_collider_component.h"

#include <format>

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/types.h>

#include "engine/components/transform_component.h"
#include "engine/core/debug.h"
#include "engine/core/physics.h"
#include "engine/entity/entity.h"

namespace hob {
    CircleColliderComponent::CircleColliderComponent(Entity& entity)
        : ColliderComponent(entity) {
    }

    std::string CircleColliderComponent::to_string() const {
        return std::format("CircleColliderComponent(entity_id = {})", get_entity().get_id());
    }

    Circle CircleColliderComponent::get_circle() const {
        return m_circle;
    }

    void CircleColliderComponent::set_circle(const Circle& circle) {
        m_circle = circle;
    }

    b2ShapeId CircleColliderComponent::create_shape(const b2ShapeDef& shape_def) {
        b2Circle b2_circle;
        b2_circle.center = Physics::vec2_to_b2Vec2(m_circle.center);
        b2_circle.radius = m_circle.radius;

        b2ShapeId shape_id = b2CreateCircleShape(get_body_id(), &shape_def, &b2_circle);
        return shape_id;
    }

    void CircleColliderComponent::debug_draw_shape(const Color& color) const {
        const TransformComponent* transform = get_entity().get_transform();
        Vector2 position = transform->get_position();
        float radians = transform->get_rotation();

        Vector2 center_world = Vector2::rotate_around(position + m_circle.center, position, radians);
        Vector2 radius_end = center_world + Vector2::right() * m_circle.radius;
        Vector2 rotated_radius_end = Vector2::rotate_around(radius_end, center_world, radians);

        debug::draw_circle(center_world, m_circle.radius, color);
        debug::draw_line(center_world, rotated_radius_end, color);
    }
}
