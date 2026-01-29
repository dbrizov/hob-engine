#include "Entity.h"

#include <fmt/base.h>

void Entity::enter_play() {
    m_is_in_play = true;
    fmt::println("enter_play (id: {})", id());
    // TODO m_components.enter_play()
}

void Entity::exit_play() {
    m_is_in_play = false;
    fmt::println("exit_play (id: {})", id());
    // TODO m_components.exit_play()
}

void Entity::tick(float delta_time) {
    // TODO m_components.tick()
}

void Entity::physics_tick(float fixed_delta_time) {
    // TODO m_components.physics_tick()
}

void Entity::render_tick(float delta_time) {
    // TODO m_components.render_tick()
}

EntityId Entity::id() const {
    return m_id;
}

void Entity::set_id(EntityId id) {
    m_id = id;
}

bool Entity::is_ticking() const {
    return m_is_ticking;
}

void Entity::set_is_ticking(bool is_ticking) {
    m_is_ticking = is_ticking;
}

bool Entity::is_in_play() const {
    return m_is_in_play;
}
