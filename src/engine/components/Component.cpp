#include "Component.h"

#include "engine/entity/Entity.h"

App* Component::get_app() const {
    assert(m_entity != nullptr);
    return m_entity->get_app();
}

Entity* Component::get_entity() const {
    return m_entity;
}

void Component::set_entity(Entity* entity) {
    m_entity = entity;
}

ComponentPriority Component::get_priority() const {
    return ComponentPriority::DEFAULT;
}

void Component::enter_play() {
}

void Component::exit_play() {
}

void Component::tick(float delta_time) {
}

void Component::physics_tick(float fixed_delta_time) {
}

void Component::render_tick(float delta_time, RenderQueue& render_queue) {
}
