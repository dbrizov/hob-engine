#include "RigidbodyComponent.h"

#include <box2d/box2d.h>

#include "TransformComponent.h"
#include "engine/core/App.h"
#include "engine/entity/Entity.h"

RigidbodyComponent::RigidbodyComponent(Entity& entity)
    : Component(entity) {
}

void RigidbodyComponent::enter_play() {
    const TransformComponent* transform = get_entity().get_transform();
    Vector2 position = transform->get_position();

    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.position = {position.x, position.y};
    body_def.rotation = b2MakeRot(transform->get_rotation_radians());

    switch (m_body_type) {
        case BodyType::STATIC:
            body_def.type = b2_staticBody;
            break;
        case BodyType::DYNAMIC:
            body_def.type = b2_dynamicBody;
            break;
        case BodyType::KINEMATIC:
            body_def.type = b2_kinematicBody;
            break;
    }

    body_def.fixedRotation = m_has_fixed_rotation;

    const PhysicsWorld& physics_world = get_app().get_physics().get_physics_world();
    m_body_id = b2CreateBody(physics_world.get_id(), &body_def);
}

void RigidbodyComponent::exit_play() {
    if (b2Body_IsValid(m_body_id)) {
        b2DestroyBody(m_body_id);
        m_body_id = b2_nullBodyId;
    }
}

b2BodyId RigidbodyComponent::get_body_id() const {
    return m_body_id;
}

bool RigidbodyComponent::has_body() const {
    return b2Body_IsValid(m_body_id);
}

BodyType RigidbodyComponent::get_body_type() const {
    return m_body_type;
}

void RigidbodyComponent::set_body_type(BodyType body_type) {
    m_body_type = body_type;
}

bool RigidbodyComponent::has_fixed_rotation() const {
    return m_has_fixed_rotation;
}

void RigidbodyComponent::set_has_fixed_rotation(bool has_fixed_rotation) {
    m_has_fixed_rotation = has_fixed_rotation;
}
