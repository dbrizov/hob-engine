#include "Physics.h"

#include <cassert>

#include "engine/components/TransformComponent.h"
#include "engine/entity/Entity.h"

Physics::Physics(uint32_t ticks_per_second, bool use_interpolation)
    : m_accumulator(0.0f)
      , m_fixed_delta_time(delta_time_from_ticks(ticks_per_second))
      , m_interpolation_fraction(0.0f)
      , m_use_interpolation(use_interpolation) {
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
            TransformComponent* transform = entity->get_transform();
            transform->set_prev_position(transform->get_position());

            entity->physics_tick(m_fixed_delta_time);
        }

        m_accumulator -= m_fixed_delta_time;
    }

    m_interpolation_fraction = m_use_interpolation ? (m_accumulator / m_fixed_delta_time) : 1.0f;
}

float Physics::delta_time_from_ticks(uint32_t ticks_per_second) {
    assert(ticks_per_second > 0 && "Division by zero");
    return 1.0f / static_cast<float>(ticks_per_second);
}
