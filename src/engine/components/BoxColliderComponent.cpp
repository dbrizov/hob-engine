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

    b2Polygon box = b2MakeBox(m_aabb.extents.x, m_aabb.extents.y);
    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.density = m_density;
    shape_def.material.friction = m_friction;
    shape_def.material.restitution = m_bounciness;
    shape_def.filter.categoryBits = m_collision_layer;
    shape_def.filter.maskBits = m_collision_mask;
    shape_def.isSensor = m_is_trigger;

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

    Vector2 top_left = position + Vector2::left() * m_aabb.extents.x + Vector2::up() * m_aabb.extents.y;
    Vector2 top_right = position + Vector2::right() * m_aabb.extents.x + Vector2::up() * m_aabb.extents.y;
    Vector2 bottom_left = position + Vector2::left() * m_aabb.extents.x + Vector2::down() * m_aabb.extents.y;
    Vector2 bottom_right = position + Vector2::right() * m_aabb.extents.x + Vector2::down() * m_aabb.extents.y;

    top_left = Vector2::rotate_around(top_left, position, rotation);
    top_right = Vector2::rotate_around(top_right, position, rotation);
    bottom_left = Vector2::rotate_around(bottom_left, position, rotation);
    bottom_right = Vector2::rotate_around(bottom_right, position, rotation);

    BodyType body_type = get_entity().get_rigidbody()->get_body_type();
    Color color;
    switch (body_type) {
        case BodyType::STATIC:
            color = Color::red();
            break;
        case BodyType::DYNAMIC:
            color = Color::green();
            break;
        case BodyType::KINEMATIC:
            color = Color::yellow();
            break;
    }

    debug::draw_line(top_left, top_right, color);
    debug::draw_line(top_right, bottom_right, color);
    debug::draw_line(bottom_right, bottom_left, color);
    debug::draw_line(bottom_left, top_left, color);
}

AABB BoxColliderComponent::get_aabb() const {
    return m_aabb;
}

void BoxColliderComponent::set_aabb(const AABB& aabb) {
    m_aabb = aabb;
}

float BoxColliderComponent::get_density() const {
    return m_density;
}

void BoxColliderComponent::set_density(float density) {
    m_density = density;
}

float BoxColliderComponent::get_friction() const {
    return m_friction;
}

void BoxColliderComponent::set_friction(float friction) {
    m_friction = friction;
}

float BoxColliderComponent::get_bounciness() const {
    return m_bounciness;
}

void BoxColliderComponent::set_bounciness(float bounciness) {
    m_bounciness = bounciness;
}

uint64_t BoxColliderComponent::get_collision_layer() const {
    return m_collision_layer;
}

void BoxColliderComponent::set_collision_layer(uint64_t collision_layer) {
    m_collision_layer = collision_layer;
}

uint64_t BoxColliderComponent::get_collision_mask() const {
    return m_collision_mask;
}

void BoxColliderComponent::set_collision_mask(uint64_t collision_mask) {
    m_collision_mask = collision_mask;
}

bool BoxColliderComponent::is_trigger() const {
    return m_is_trigger;
}

void BoxColliderComponent::set_trigger(bool trigger) {
    m_is_trigger = trigger;
}
