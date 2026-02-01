#include "Entity.h"

void Entity::enter_play() {
    m_is_in_play = true;
    for (auto& component : m_components) {
        component->enter_play();
    }
}

void Entity::exit_play() {
    m_is_in_play = false;
    for (auto& component : m_components) {
        component->exit_play();
    }
}

void Entity::tick(float delta_time) {
    for (auto& component : m_components) {
        component->tick(delta_time);
    }
}

void Entity::physics_tick(float fixed_delta_time) {
    for (auto& component : m_components) {
        component->physics_tick(fixed_delta_time);
    }
}

void Entity::render_tick(float delta_time) {
    for (auto& component : m_components) {
        component->render_tick(delta_time);
    }
}

EntityId Entity::get_id() const {
    return m_id;
}

void Entity::set_id(EntityId id) {
    m_id = id;
}

bool Entity::is_in_play() const {
    return m_is_in_play;
}

bool Entity::is_ticking() const {
    return m_is_ticking;
}

void Entity::set_is_ticking(bool is_ticking) {
    m_is_ticking = is_ticking;
}
