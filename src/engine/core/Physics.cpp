#include "Physics.h"

#include <cassert>

#include "App.h"
#include "engine/components/ColliderComponent.h"
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

        // Dispatch events
        dispatch_collision_events();
        dispatch_trigger_events();

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

const PhysicsWorld& Physics::get_physics_world() const {
    return m_physics_world;
}

float Physics::get_fixed_delta_time() const {
    return m_fixed_delta_time;
}

float Physics::get_interpolation_fraction() const {
    return m_interpolation_fraction;
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

void Physics::dispatch_collision_events() const {
    b2ContactEvents contact_events = b2World_GetContactEvents(m_physics_world.get_id());

    // Dispatch on_collision_enter
    for (int i = 0; i < contact_events.beginCount; ++i) {
        const b2ContactBeginTouchEvent& ev = contact_events.beginEvents[i];

        if (!b2Shape_IsValid(ev.shapeIdA) || !b2Shape_IsValid(ev.shapeIdB)) {
            continue;
        }

        const auto* collider_a = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.shapeIdA));
        const auto* collider_b = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.shapeIdB));

        Entity& entity_a = collider_a->get_entity();
        Entity& entity_b = collider_b->get_entity();

        entity_a.on_collision_enter(collider_b);
        entity_b.on_collision_enter(collider_a);
    }

    // Dispatch on_collision_exit
    for (int i = 0; i < contact_events.endCount; ++i) {
        const b2ContactEndTouchEvent& ev = contact_events.endEvents[i];

        if (!b2Shape_IsValid(ev.shapeIdA) || !b2Shape_IsValid(ev.shapeIdB)) {
            continue;
        }

        const auto* collider_a = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.shapeIdA));
        const auto* collider_b = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.shapeIdB));

        Entity& entity_a = collider_a->get_entity();
        Entity& entity_b = collider_b->get_entity();

        entity_a.on_collision_exit(collider_b);
        entity_b.on_collision_exit(collider_a);
    }
}

void Physics::dispatch_trigger_events() const {
    b2SensorEvents sensor_events = b2World_GetSensorEvents(m_physics_world.get_id());

    // Dispatch on_trigger_enter
    for (int i = 0; i < sensor_events.beginCount; ++i) {
        const b2SensorBeginTouchEvent& ev = sensor_events.beginEvents[i];

        if (!b2Shape_IsValid(ev.sensorShapeId) || !b2Shape_IsValid(ev.visitorShapeId)) {
            continue;
        }

        const auto* trigger_collider = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.sensorShapeId));
        const auto* visitor_collider = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.visitorShapeId));

        Entity& trigger_entity = trigger_collider->get_entity();
        Entity& visitor_entity = visitor_collider->get_entity();

        trigger_entity.on_trigger_enter(visitor_collider);
        visitor_entity.on_trigger_enter(trigger_collider);
    }

    // Dispatch on_trigger_exit
    for (int i = 0; i < sensor_events.endCount; ++i) {
        const b2SensorEndTouchEvent& ev = sensor_events.endEvents[i];

        if (!b2Shape_IsValid(ev.sensorShapeId) || !b2Shape_IsValid(ev.visitorShapeId)) {
            continue;
        }

        const auto* trigger_collider = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.sensorShapeId));
        const auto* visitor_collider = static_cast<ColliderComponent*>(b2Shape_GetUserData(ev.visitorShapeId));

        Entity& trigger_entity = trigger_collider->get_entity();
        Entity& visitor_entity = visitor_collider->get_entity();

        trigger_entity.on_trigger_exit(visitor_collider);
        visitor_entity.on_trigger_exit(trigger_collider);
    }
}

float Physics::delta_time_from_ticks(uint32_t ticks_per_second) {
    assert(ticks_per_second > 0 && "Division by zero");
    return 1.0f / static_cast<float>(ticks_per_second);
}
