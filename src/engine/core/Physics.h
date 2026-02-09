#ifndef CPP_PLATFORMER_PHYSICS_H
#define CPP_PLATFORMER_PHYSICS_H
#include <cstdint>
#include <vector>


class Entity;


class Physics {
    float m_accumulator;
    float m_fixed_delta_time;
    float m_interpolation_fraction;
    bool m_use_interpolation;

public:
    Physics(uint32_t ticks_per_second, bool use_interpolation);

    float get_fixed_delta_time() const;
    float get_interpolation_fraction() const;

    void tick_entities(float frame_delta_time, const std::vector<Entity*>& entities);

private:
    static float delta_time_from_ticks(uint32_t ticks_per_second);
};


#endif //CPP_PLATFORMER_PHYSICS_H
