#include "BoxColliderComponent.h"

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/types.h>

#include "RigidbodyComponent.h"
#include "TransformComponent.h"
#include "engine/core/Debug.h"
#include "engine/entity/Entity.h"

BoxColliderComponent::BoxColliderComponent(Entity& entity)
    : Component(entity) {
}

void BoxColliderComponent::enter_play() {
    const RigidbodyComponent* rigidbody = get_entity().get_rigidbody();
    assert(rigidbody != nullptr && rigidbody->has_body() && "BoxCollider requires a Rigidbody to function");

    b2Polygon box = b2MakeBox(m_half_width, m_half_height);

    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.density = m_density;
    shape_def.material.friction = m_friction;
    shape_def.material.restitution = m_restitution;
    shape_def.isSensor = m_is_trigger;

    shape_def.filter.categoryBits = m_category_bits;
    shape_def.filter.maskBits = m_mask_bits;

    m_shape_id = b2CreatePolygonShape(rigidbody->get_body_id(), &shape_def, &box);
}

void BoxColliderComponent::exit_play() {
    if (b2Shape_IsValid(m_shape_id)) {
        b2DestroyShape(m_shape_id, false);
        m_shape_id = b2_nullShapeId;
    }
}

void BoxColliderComponent::render_tick(float delta_time, RenderQueue& render_queue) {
    const TransformComponent* transform = get_entity().get_transform();
    Vector2 position = transform->get_position();
    float rotation = transform->get_rotation_degrees();

    Vector2 top_left = position + Vector2::left() * m_half_width + Vector2::up() * m_half_height;
    Vector2 top_right = position + Vector2::right() * m_half_width + Vector2::up() * m_half_height;
    Vector2 bottom_left = position + Vector2::left() * m_half_width + Vector2::down() * m_half_height;
    Vector2 bottom_right = position + Vector2::right() * m_half_width + Vector2::down() * m_half_height;

    top_left = Vector2::rotate_around(top_left, position, rotation);
    top_right = Vector2::rotate_around(top_right, position, rotation);
    bottom_left = Vector2::rotate_around(bottom_left, position, rotation);
    bottom_right = Vector2::rotate_around(bottom_right, position, rotation);

    Color color = get_entity().get_rigidbody()->get_body_type() == BodyType::STATIC ? Color::yellow() : Color::green();
    debug::draw_line(top_left, top_right, color);
    debug::draw_line(top_right, bottom_right, color);
    debug::draw_line(bottom_right, bottom_left, color);
    debug::draw_line(bottom_left, top_left, color);
}
