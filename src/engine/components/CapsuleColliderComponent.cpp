#include "CapsuleColliderComponent.h"

#include <cassert>
#include <box2d/types.h>

#include "RigidbodyComponent.h"
#include "TransformComponent.h"
#include "engine/core/Debug.h"
#include "engine/core/Physics.h"
#include "engine/core/Render.h"
#include "engine/entity/Entity.h"
#include "engine/math/Constants.h"

CapsuleColliderComponent::CapsuleColliderComponent(Entity& entity)
    : Component(entity) {
}

void CapsuleColliderComponent::enter_play() {
    const RigidbodyComponent* rigidbody = get_entity().get_rigidbody();
    assert(rigidbody != nullptr && rigidbody->has_body() && "CapsuleCollider requires a Rigidbody to function");

    b2Capsule b2_capsule;
    b2_capsule.center1 = Physics::vec2_to_b2Vec2(m_capsule.center_a);
    b2_capsule.center2 = Physics::vec2_to_b2Vec2(m_capsule.center_b);
    b2_capsule.radius = m_capsule.radius;

    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.density = m_density;
    shape_def.material.friction = m_friction;
    shape_def.material.restitution = m_bounciness;
    shape_def.filter.categoryBits = m_collision_layer;
    shape_def.filter.maskBits = m_collision_mask;
    shape_def.isSensor = m_is_trigger;

    m_shape_id = b2CreateCapsuleShape(rigidbody->get_body_id(), &shape_def, &b2_capsule);
}

void CapsuleColliderComponent::exit_play() {
    if (b2Shape_IsValid(m_shape_id)) {
        b2DestroyShape(m_shape_id, false);
        m_shape_id = b2_nullShapeId;
    }
}

void CapsuleColliderComponent::render_tick(float delta_time, RenderQueue& render_queue) {
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

    // Choose a color
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

    // Draw
    debug::draw_line(p1, p2, color);
    debug::draw_line(p3, p4, color);

    debug::draw_circle(c1_world, m_capsule.radius, color);
    debug::draw_circle(c2_world, m_capsule.radius, color);
}

b2ShapeId CapsuleColliderComponent::get_shape_id() const {
    return m_shape_id;
}

Capsule CapsuleColliderComponent::get_capsule() const {
    return m_capsule;
}

void CapsuleColliderComponent::set_capsule(const Capsule& capsule) {
    m_capsule = capsule;
}

float CapsuleColliderComponent::get_density() const {
    return m_density;
}

void CapsuleColliderComponent::set_density(float density) {
    m_density = density;
}

float CapsuleColliderComponent::get_friction() const {
    return m_friction;
}

void CapsuleColliderComponent::set_friction(float friction) {
    m_friction = friction;
}

float CapsuleColliderComponent::get_bounciness() const {
    return m_bounciness;
}

void CapsuleColliderComponent::set_bounciness(float bounciness) {
    m_bounciness = bounciness;
}

uint64_t CapsuleColliderComponent::get_collision_layer() const {
    return m_collision_layer;
}

void CapsuleColliderComponent::set_collision_layer(uint64_t collision_layer) {
    m_collision_layer = collision_layer;
}

uint64_t CapsuleColliderComponent::get_collision_mask() const {
    return m_collision_mask;
}

void CapsuleColliderComponent::set_collision_mask(uint64_t collision_mask) {
    m_collision_mask = collision_mask;
}

bool CapsuleColliderComponent::is_trigger() const {
    return m_is_trigger;
}

void CapsuleColliderComponent::set_trigger(bool trigger) {
    m_is_trigger = trigger;
}
