#include "BoxColliderComponent.h"

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/types.h>

#include "engine/components/TransformComponent.h"
#include "engine/core/Debug.h"
#include "engine/entity/Entity.h"

BoxColliderComponent::BoxColliderComponent(Entity& entity)
    : ColliderComponent(entity) {
}

AABB BoxColliderComponent::get_aabb() const {
    return m_aabb;
}

void BoxColliderComponent::set_aabb(const AABB& aabb) {
    m_aabb = aabb;
}

b2ShapeId BoxColliderComponent::create_shape(const b2ShapeDef& shape_def) {
    b2Polygon box = b2MakeBox(m_aabb.extents.x, m_aabb.extents.y);
    b2ShapeId shape_id = b2CreatePolygonShape(get_body_id(), &shape_def, &box);
    return shape_id;
}

void BoxColliderComponent::debug_draw_shape(const Color& color) const {
    const TransformComponent* transform = get_entity().get_transform();
    Vector2 position = transform->get_position();
    float rotation = transform->get_rotation_degrees();

    Vector2 top_left = position + Vector2::left() * m_aabb.extents.x + Vector2::up() * m_aabb.extents.y;
    Vector2 top_right = position + Vector2::right() * m_aabb.extents.x + Vector2::up() * m_aabb.extents.y;
    Vector2 bottom_left = position + Vector2::left() * m_aabb.extents.x + Vector2::down() * m_aabb.extents.y;
    Vector2 bottom_right = position + Vector2::right() * m_aabb.extents.x + Vector2::down() * m_aabb.extents.y;

    top_left = Vector2::rotate_around(top_left, position, rotation);
    top_right = Vector2::rotate_around(top_right, position, rotation);
    bottom_left = Vector2::rotate_around(bottom_left, position, rotation);
    bottom_right = Vector2::rotate_around(bottom_right, position, rotation);

    debug::draw_line(top_left, top_right, color);
    debug::draw_line(top_right, bottom_right, color);
    debug::draw_line(bottom_right, bottom_left, color);
    debug::draw_line(bottom_left, top_left, color);
}
