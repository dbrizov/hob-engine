#include "Physics.h"

#include <cassert>

#include "engine/entity/Entity.h"

Physics::Physics(uint32_t ticks_per_second, bool interpolation)
    : m_fixed_delta_time(fixed_delta_time_from_ticks_per_second(ticks_per_second))
      , m_interpolation(interpolation)
      , m_interpolation_fraction(0.0f)
      , m_accumulator(0.0f) {
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
        for (Entity* entity : entities) {
            entity->physics_tick(m_fixed_delta_time);
        }

        m_accumulator -= m_fixed_delta_time;
    }

    m_interpolation_fraction = m_interpolation ? (m_accumulator / m_fixed_delta_time) : 1.0f;
}

float Physics::fixed_delta_time_from_ticks_per_second(uint32_t ticks_per_second) {
    assert(ticks_per_second > 0);
    return 1.0f / static_cast<float>(ticks_per_second);
}
