#include "CharacterBodyComponent.h"

#include <box2d/box2d.h>

#include "TransformComponent.h"
#include "engine/components/CapsuleColliderComponent.h"
#include "engine/components/RigidbodyComponent.h"
#include "engine/core/App.h"
#include "engine/core/Physics.h"
#include "engine/entity/Entity.h"
#include "engine/math/Constants.h"

CharacterBodyComponent::CharacterBodyComponent(Entity& entity)
    : Component(entity) {
    m_rigidbody = entity.add_component<RigidbodyComponent>();
    m_rigidbody->set_body_type(BodyType::KINEMATIC);
    m_rigidbody->set_fixed_rotation(true);

    m_capsule_collider = entity.add_component<CapsuleColliderComponent>();
}

int CharacterBodyComponent::get_priority() const {
    return component_priority::CP_CHARACTER_BODY;
}

void CharacterBodyComponent::move_and_slide(const Vector2& desired_velocity, float delta_time) {
    Vector2 delta_pos = desired_velocity * delta_time;
    if (delta_pos.length_sqr() < EPSILON) {
        return;
    }

    // Filters:
    // - collide against world + movers if you want (later)
    // - cast against world only to allow soft character-character
    b2QueryFilter collision_filter = b2DefaultQueryFilter();
    collision_filter.categoryBits = m_capsule_collider->get_collision_layer();
    collision_filter.maskBits = m_capsule_collider->get_collision_mask();

    // TODO Ignore other characters
    b2QueryFilter cast_filter = b2DefaultQueryFilter();
    cast_filter.categoryBits = m_capsule_collider->get_collision_layer();
    cast_filter.maskBits = m_capsule_collider->get_collision_mask();

    // Solver data
    b2BodyId body_id = m_rigidbody->get_body_id();
    b2WorldId world_id = get_app().get_physics().get_physics_world().get_id();

    Capsule local_capsule = m_capsule_collider->get_capsule();

    b2Vec2 b2_start_pos = b2Body_GetPosition(body_id);
    b2Rot b2_rot = b2Body_GetRotation(body_id);
    float rot_degrees = Physics::b2Rot_to_radians(b2_rot) * RAD_TO_DEG;

    b2Vec2 b2_current_pos = b2_start_pos;
    b2Vec2 b2_delta_pos = Physics::vec2_to_b2Vec2(delta_pos);
    b2Vec2 b2_target_pos = b2Add(b2_current_pos, b2_delta_pos);

    for (int i = 0; i < SOLVER_MAX_ITERATIONS; ++i) {
        m_solver_planes_count = 0;

        // Build capsule at current position
        b2Capsule mover = make_world_capsule(local_capsule, Physics::b2Vec2_to_vec2(b2_current_pos), rot_degrees);

        // 1) Gather planes at current position
        b2World_CollideMover(world_id, &mover, collision_filter, plane_result_callback, this);

        // 2) Solve for the "best" move toward the target given those planes
        b2Vec2 b2_desired_delta = b2Sub(b2_target_pos, b2_current_pos);
        b2PlaneSolverResult b2_solver_result = b2SolvePlanes(b2_desired_delta, m_solver_planes, m_solver_planes_count);
        b2Vec2 b2_solver_translation = b2_solver_result.translation;

        // 3) Cast to make that translation continuous (no tunneling)
        float fraction = b2World_CastMover(world_id, &mover, b2_solver_translation, cast_filter);
        b2Vec2 b2_delta = b2MulSV(fraction, b2_solver_translation);

        b2_current_pos = b2Add(b2_current_pos, b2_delta);

        if (b2Dot(b2_delta, b2_delta) < SOLVER_DISTANCE_TOLERANCE * SOLVER_DISTANCE_TOLERANCE) {
            break;
        }
    }

    // Apply final position to the kinematic body
    b2Transform b2_transform = b2Body_GetTransform(body_id);
    b2_transform.p = b2_current_pos;
    b2Body_SetTransform(body_id, b2_transform.p, b2_transform.q);
}

b2Capsule CharacterBodyComponent::make_world_capsule(const Capsule& local_capsule,
                                                     const Vector2& position,
                                                     float rotation_degrees) {
    Vector2 c1_world = Vector2::rotate_around(position + local_capsule.center_a, position, rotation_degrees);
    Vector2 c2_world = Vector2::rotate_around(position + local_capsule.center_b, position, rotation_degrees);

    b2Capsule b2_capsule;
    b2_capsule.center1 = Physics::vec2_to_b2Vec2(c1_world);
    b2_capsule.center2 = Physics::vec2_to_b2Vec2(c2_world);
    b2_capsule.radius = local_capsule.radius;

    return b2_capsule;
}

bool CharacterBodyComponent::plane_result_callback(b2ShapeId shape_id,
                                                   const b2PlaneResult* plane_result,
                                                   void* context) {
    auto* self = static_cast<CharacterBodyComponent*>(context);
    if (!self || !plane_result || !plane_result->hit) {
        return true; // skip but keep searching
    }

    // Ignore my own collider
    if (shape_id.index1 == self->m_capsule_collider->get_shape_id().index1) {
        return true; // skip but keep searching
    }

    // If full, stop searching
    if (self->m_solver_planes_count >= SOLVER_PLANES_CAPACITY) {
        return false;
    }

    // TODO optional UserData for pushing objects
    float push_limit = MAX_FLOAT;
    bool clip_velocity = true;

    self->m_solver_planes[self->m_solver_planes_count] = {plane_result->plane, push_limit, 0.0f, clip_velocity};
    self->m_solver_planes_count += 1;

    return true; // keep searching
}
