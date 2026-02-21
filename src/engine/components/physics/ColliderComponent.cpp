#include "ColliderComponent.h"

#include <box2d/box2d.h>

#include "RigidbodyComponent.h"
#include "engine/core/Render.h"
#include "engine/entity/Entity.h"

namespace hob {
    ColliderComponent::ColliderComponent(Entity& entity)
        : Component(entity) {
    }

    void ColliderComponent::enter_play() {
        const RigidbodyComponent* rigidbody = get_entity().get_rigidbody();
        assert(rigidbody != nullptr && rigidbody->has_body() && "Collider requires a Rigidbody to function");

        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.density = m_density;
        shape_def.material.friction = m_friction;
        shape_def.material.restitution = m_bounciness;
        shape_def.filter.categoryBits = m_collision_layer;
        shape_def.filter.maskBits = m_collision_mask;
        shape_def.isSensor = m_is_trigger;

        m_shape_id = create_shape(shape_def);

        b2Shape_SetUserData(m_shape_id, this);
        b2Shape_EnableSensorEvents(m_shape_id, true);
        b2Shape_EnableContactEvents(m_shape_id, !m_is_trigger); // enable contact events if it's NOT a sensor (trigger)
    }

    void ColliderComponent::exit_play() {
        if (b2Shape_IsValid(m_shape_id)) {
            b2DestroyShape(m_shape_id, false);
            m_shape_id = b2_nullShapeId;
        }
    }

    void ColliderComponent::debug_draw_tick(float delta_time) {
        Color color;
        if (m_is_trigger) {
            color = Color::cyan();
        }
        else {
            BodyType body_type = get_entity().get_rigidbody()->get_body_type();
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
        }

        debug_draw_shape(color);
    }

    b2BodyId ColliderComponent::get_body_id() const {
        b2BodyId body_id = get_entity().get_rigidbody()->get_body_id();
        return body_id;
    }

    b2ShapeId ColliderComponent::get_shape_id() const {
        return m_shape_id;
    }

    float ColliderComponent::get_density() const {
        return m_density;
    }

    void ColliderComponent::set_density(float density) {
        m_density = density;
    }

    float ColliderComponent::get_friction() const {
        return m_friction;
    }

    void ColliderComponent::set_friction(float friction) {
        m_friction = friction;
    }

    float ColliderComponent::get_bounciness() const {
        return m_bounciness;
    }

    void ColliderComponent::set_bounciness(float bounciness) {
        m_bounciness = bounciness;
    }

    uint64_t ColliderComponent::get_collision_layer() const {
        return m_collision_layer;
    }

    void ColliderComponent::set_collision_layer(uint64_t collision_layer) {
        m_collision_layer = collision_layer;
    }

    uint64_t ColliderComponent::get_collision_mask() const {
        return m_collision_mask;
    }

    void ColliderComponent::set_collision_mask(uint64_t collision_mask) {
        m_collision_mask = collision_mask;
    }

    bool ColliderComponent::is_trigger() const {
        return m_is_trigger;
    }

    void ColliderComponent::set_trigger(bool trigger) {
        m_is_trigger = trigger;
    }
}
