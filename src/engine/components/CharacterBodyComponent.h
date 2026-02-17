#ifndef HOB_ENGINE_CHARACTERBODYCOMPONENT_H
#define HOB_ENGINE_CHARACTERBODYCOMPONENT_H
#include <box2d/collision.h>
#include <box2d/id.h>

#include "Component.h"
#include "engine/math/Vector2.h"


struct Capsule;
class CapsuleColliderComponent;
class RigidbodyComponent;


class CharacterBodyComponent : public Component {
    RigidbodyComponent* m_rigidbody = nullptr;
    CapsuleColliderComponent* m_capsule_collider = nullptr;

    // Collision solver params
    static constexpr int SOLVER_MAX_ITERATIONS = 4;
    static constexpr float SOLVER_DISTANCE_TOLERANCE = 0.01f;
    static constexpr int SOLVER_PLANES_CAPACITY = 8;
    int m_solver_planes_count = 0;
    b2CollisionPlane m_solver_planes[SOLVER_PLANES_CAPACITY] = {};

public:
    explicit CharacterBodyComponent(Entity& entity);

    virtual int get_priority() const override;

    uint64_t get_collision_layer() const;
    void set_collision_layer(uint64_t collision_layer);

    uint64_t get_collision_mask() const;
    void set_collision_mask(uint64_t collision_mask);

    void move_and_slide(const Vector2& desired_velocity, float delta_time);

private:
    static b2Capsule make_world_capsule(const Capsule& local_capsule, const Vector2& position, float rotation_degrees);
    static bool plane_result_callback(b2ShapeId other_shape_id, const b2PlaneResult* plane_result, void* context);
};


#endif //HOB_ENGINE_CHARACTERBODYCOMPONENT_H
