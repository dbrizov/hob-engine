#include "Physics.h"

#include <cassert>
#include <fmt/format.h>

#include "engine/components/RigidbodyComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/entity/Entity.h"
#include "engine/math/Math.h"

PhysicsWorld::PhysicsWorld()
    : m_id(b2_nullWorldId) {
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = b2Vec2(0, -9.81f);
    m_id = b2CreateWorld(&world_def);
}

PhysicsWorld::~PhysicsWorld() {
    if (b2World_IsValid(m_id)) {
        b2DestroyWorld(m_id);
    }

    m_id = b2_nullWorldId;
}

void PhysicsWorld::tick(float fixed_delta_time, uint32_t sub_steps) {
    b2World_Step(m_id, fixed_delta_time, static_cast<int>(sub_steps));
}

b2WorldId PhysicsWorld::get_id() const {
    return m_id;
}

Physics::Physics(uint32_t ticks_per_second, uint32_t sub_steps_per_tick, bool use_interpolation)
    : m_physics_world()
      , m_accumulator(0.0f)
      , m_fixed_delta_time(delta_time_from_ticks(ticks_per_second))
      , m_sub_steps_per_tick(sub_steps_per_tick)
      , m_interpolation_fraction(0.0f)
      , m_use_interpolation(use_interpolation) {
}

const PhysicsWorld& Physics::get_physics_world() const {
    return m_physics_world;
}

float Physics::get_fixed_delta_time() const {
    return m_fixed_delta_time;
}

float Physics::get_interpolation_fraction() const {
    return m_interpolation_fraction;
}

void Physics::tick_entities(float frame_delta_time, const std::vector<Entity*>& entities) {
    m_accumulator += frame_delta_time;
    while (m_accumulator >= m_fixed_delta_time) {
        // Let components apply forces / set kinematic velocities
        for (Entity* entity : entities) {
            entity->physics_tick(m_fixed_delta_time);
        }

        // Tick the physics world
        m_physics_world.tick(m_fixed_delta_time, m_sub_steps_per_tick);

        // Sync transforms for all rigidbodies
        for (Entity* entity : entities) {
            // Save previous position for physics interpolation
            TransformComponent* transform = entity->get_transform();
            transform->set_prev_position(transform->get_position());

            const RigidbodyComponent* rigidbody = entity->get_rigidbody();
            if (!rigidbody || !rigidbody->has_body()) {
                continue;
            }

            b2Vec2 position = b2Body_GetPosition(rigidbody->get_body_id());
            b2Rot rotation  = b2Body_GetRotation(rigidbody->get_body_id());
            float rotation_radians = std::atan2(rotation.s, rotation.c);

            transform->set_position(Vector2(position.x, position.y));
            transform->set_rotation_radians(rotation_radians);
        }

        m_accumulator -= m_fixed_delta_time;
    }

    m_interpolation_fraction = m_use_interpolation ? (m_accumulator / m_fixed_delta_time) : 1.0f;
}

float Physics::delta_time_from_ticks(uint32_t ticks_per_second) {
    assert(ticks_per_second > 0 && "Division by zero");
    return 1.0f / static_cast<float>(ticks_per_second);
}
