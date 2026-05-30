#include "circle_collider_component.h"

#include <cmath>
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

    Circle CircleColliderComponent::get_scaled_circle() const {
        return scale_circle(m_circle, get_initial_scale());
    }

    b2ShapeId CircleColliderComponent::create_shape(const b2ShapeDef& shape_def, const Vector2& scale) {
        Circle scaled = scale_circle(m_circle, scale);

        b2Circle b2_circle;
        b2_circle.center = Physics::vec2_to_b2Vec2(scaled.center);
        b2_circle.radius = scaled.radius;

        b2ShapeId shape_id = b2CreateCircleShape(get_body_id(), &shape_def, &b2_circle);
        return shape_id;
    }

    void CircleColliderComponent::debug_draw_shape(const Color& color, const Vector2& scale) const {
        const TransformComponent* transform = get_entity().get_transform();
        Vector2 position = transform->get_position();
        float radians = transform->get_rotation();

        Circle scaled = scale_circle(m_circle, scale);

        Vector2 center_world = Vector2::rotate_around(position + scaled.center, position, radians);
        Vector2 radius_end = center_world + Vector2::right() * scaled.radius;
        Vector2 rotated_radius_end = Vector2::rotate_around(radius_end, center_world, radians);

        debug::draw_circle(center_world, scaled.radius, color);
        debug::draw_line(center_world, rotated_radius_end, color);
    }

    Circle CircleColliderComponent::scale_circle(const Circle& local, const Vector2& scale) {
        Vector2 center(local.center.x * scale.x, local.center.y * scale.y);
        float radius = local.radius * std::max(std::abs(scale.x), std::abs(scale.y));
        return Circle(center, radius);
    }
}
