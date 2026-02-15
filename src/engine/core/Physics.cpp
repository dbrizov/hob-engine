#include "Physics.h"

#include <cassert>

#include "App.h"
#include "engine/components/RigidbodyComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/entity/Entity.h"

PhysicsWorld::PhysicsWorld(const Vector2& gravity)
    : m_id(b2_nullWorldId) {
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = Physics::vec2_to_b2Vec2(gravity);
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

Physics::Physics(const PhysicsConfig& physics_config)
    : m_physics_world(physics_config.gravity)
      , m_accumulator(0.0f)
      , m_fixed_delta_time(delta_time_from_ticks(physics_config.ticks_per_second))
      , m_sub_steps_per_tick(physics_config.sub_steps_per_tick)
      , m_interpolation_fraction(0.0f)
      , m_interpolation_enabled(physics_config.interpolation_enabled) {
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
        // TODO Fix Physics interpolation - set previous position of entities

        // Let components apply forces / set kinematic velocities
        for (Entity* entity : entities) {
            entity->physics_tick(m_fixed_delta_time);
        }

        // Tick the physics world
        m_physics_world.tick(m_fixed_delta_time, m_sub_steps_per_tick);

        // Sync transforms for all rigidbodies
        for (Entity* entity : entities) {
            const RigidbodyComponent* rigidbody = entity->get_rigidbody();
            b2Vec2 b2_position = b2Body_GetPosition(rigidbody->get_body_id());
            b2Rot b2_rotation = b2Body_GetRotation(rigidbody->get_body_id());

            Vector2 position = Physics::b2Vec2_to_vec2(b2_position);
            float rotation_radians = Physics::b2Rot_to_radians(b2_rotation);

            TransformComponent* transform = entity->get_transform();
            transform->set_position(position);
            transform->set_rotation_radians(rotation_radians);
        }

        m_accumulator -= m_fixed_delta_time;
    }

    m_interpolation_fraction = m_interpolation_enabled ? (m_accumulator / m_fixed_delta_time) : 1.0f;
}

Vector2 Physics::b2Vec2_to_vec2(const b2Vec2& vec) {
    return Vector2(vec.x, vec.y);
}

b2Vec2 Physics::vec2_to_b2Vec2(const Vector2& vec) {
    return b2Vec2(vec.x, vec.y);
}

float Physics::b2Rot_to_radians(const b2Rot& rot) {
    return std::atan2(rot.s, rot.c);
}

b2Rot Physics::radians_to_b2Rot(float radians) {
    return b2MakeRot(radians);
}

float Physics::delta_time_from_ticks(uint32_t ticks_per_second) {
    assert(ticks_per_second > 0 && "Division by zero");
    return 1.0f / static_cast<float>(ticks_per_second);
}
