#ifndef HOB_ENGINE_PHYSICS_H
#define HOB_ENGINE_PHYSICS_H
#include <cstdint>
#include <vector>
#include <box2d/box2d.h>


class Entity;


class PhysicsWorld {
    b2WorldId m_id;

public:
    PhysicsWorld();
    ~PhysicsWorld();

    void tick(float fixed_delta_time, uint32_t sub_steps = 4);

    b2WorldId get_id() const;
};


class Physics {
    PhysicsWorld m_physics_world;
    float m_accumulator;
    float m_fixed_delta_time;
    uint32_t m_sub_steps_per_tick;
    float m_interpolation_fraction;
    bool m_use_interpolation;

public:
    Physics(uint32_t ticks_per_second, uint32_t sub_steps_per_tick, bool use_interpolation);

    const PhysicsWorld& get_physics_world() const;

    float get_fixed_delta_time() const;
    float get_interpolation_fraction() const;

    void tick_entities(float frame_delta_time, const std::vector<Entity*>& entities);

private:
    static float delta_time_from_ticks(uint32_t ticks_per_second);
};


#endif //HOB_ENGINE_PHYSICS_H
