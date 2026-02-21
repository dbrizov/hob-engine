#include "Entity.h"

#include "engine/components/physics/RigidbodyComponent.h"
#include "engine/components/TransformComponent.h"

namespace hob {
    Entity::Entity(App& app)
        : m_app(app) {
    }

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

    void Entity::debug_draw_tick(float delta_time) {
        for (auto& component : m_components) {
            component->debug_draw_tick(delta_time);
        }
    }

    void Entity::on_collision_enter(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_collision_enter(other_collider);
        }
    }

    void Entity::on_collision_exit(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_collision_exit(other_collider);
        }
    }

    void Entity::on_trigger_enter(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_trigger_enter(other_collider);
        }
    }

    void Entity::on_trigger_exit(const ColliderComponent* other_collider) {
        for (auto& component : m_components) {
            component->on_trigger_exit(other_collider);
        }
    }

    App& Entity::get_app() const {
        return m_app;
    }

    EntityId Entity::get_id() const {
        return m_id;
    }

    void Entity::set_id(EntityId id) {
        m_id = id;
    }

    int Entity::get_priority() const {
        return m_priority;
    }

    void Entity::set_priority(int priority) {
        m_priority = priority;
    }

    bool Entity::is_in_play() const {
        return m_is_in_play;
    }

    bool Entity::is_ticking() const {
        return m_is_ticking;
    }

    void Entity::set_ticking(bool is_ticking) {
        m_is_ticking = is_ticking;
    }

    TransformComponent* Entity::get_transform() const {
        if (m_transform == nullptr) {
            m_transform = get_component<TransformComponent>();
        }

        return m_transform;
    }

    RigidbodyComponent* Entity::get_rigidbody() const {
        if (m_rigidbody == nullptr) {
            m_rigidbody = get_component<RigidbodyComponent>();
        }

        return m_rigidbody;
    }
}
