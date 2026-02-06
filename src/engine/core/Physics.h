#ifndef CPP_PLATFORMER_PHYSICS_H
#define CPP_PLATFORMER_PHYSICS_H
#include <cstdint>
#include <vector>


class Entity;


class Physics {
    float m_fixed_delta_time;
    bool m_interpolation;
    float m_interpolation_fraction;
    float m_accumulator;

public:
    Physics(uint32_t ticks_per_second, bool interpolation);

    float get_fixed_delta_time() const;
    float get_interpolation_fraction() const;

    void tick_entities(float frame_delta_time, const std::vector<Entity*>& entities);

private:
    static float fixed_delta_time_from_ticks_per_second(uint32_t ticks_per_second);
};


#endif //CPP_PLATFORMER_PHYSICS_H
