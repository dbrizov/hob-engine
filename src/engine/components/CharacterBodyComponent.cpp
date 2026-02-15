#include "CharacterBodyComponent.h"

#include <box2d/box2d.h>

#include "TransformComponent.h"
#include "engine/components/BoxColliderComponent.h"
#include "engine/components/RigidbodyComponent.h"
#include "engine/core/Physics.h"
#include "engine/entity/Entity.h"

CharacterBodyComponent::CharacterBodyComponent(Entity& entity)
    : Component(entity) {
    m_rigidbody = entity.add_component<RigidbodyComponent>();
    m_rigidbody->set_body_type(BodyType::KINEMATIC);
    m_rigidbody->set_fixed_rotation(true);

    m_box_collider = entity.add_component<BoxColliderComponent>();
    m_box_collider->set_aabb(AABB(Vector2::zero(), Vector2(0.25f, 0.5f)));
}

int CharacterBodyComponent::get_priority() const {
    return component_priority::CP_CHARACTER_BODY;
}

Vector2 CharacterBodyComponent::get_velocity() const {
    b2Vec2 b2_velocity = b2Body_GetLinearVelocity(m_rigidbody->get_body_id());
    Vector2 velocity = Physics::b2Vec2_to_vec2(b2_velocity);

    return velocity;
}

void CharacterBodyComponent::set_velocity(const Vector2& velocity) {
    b2Vec2 b2_velocity = b2Vec2(velocity.x, velocity.y);
    b2Body_SetLinearVelocity(m_rigidbody->get_body_id(), b2_velocity);
}
