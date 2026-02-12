#include "Component.h"

#include "engine/entity/Entity.h"

Component::Component(Entity& entity)
    : m_entity(entity) {
}

App& Component::get_app() const {
    return m_entity.get_app();
}

Entity& Component::get_entity() const {
    return m_entity;
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
