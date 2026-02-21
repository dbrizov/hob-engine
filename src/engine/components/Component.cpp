#include "Component.h"

#include "engine/entity/Entity.h"

namespace hob {
    Component::Component(Entity& entity)
        : m_entity(entity) {
    }

    App& Component::get_app() const {
        return m_entity.get_app();
    }

    Entity& Component::get_entity() const {
        return m_entity;
    }

    int Component::get_priority() const {
        return component_priority::CP_DEFAULT;
    }

    void Component::enter_play() {
    }

    void Component::exit_play() {
    }

    void Component::tick(float delta_time) {
    }

    void Component::physics_tick(float fixed_delta_time) {
    }

    void Component::debug_draw_tick(float delta_time) {
    }

    void Component::on_collision_enter(const ColliderComponent* other_collider) {
    }

    void Component::on_collision_exit(const ColliderComponent* other_collider) {
    }

    void Component::on_trigger_enter(const ColliderComponent* other_collider) {
    }

    void Component::on_trigger_exit(const ColliderComponent* other_collider) {
    }
}
