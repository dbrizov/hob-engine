#ifndef CPP_PLATFORMER_ENTITY_H
#define CPP_PLATFORMER_ENTITY_H
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "engine/components/Component.h"


using EntityId = uint32_t;

template<typename T>
concept ComponentType = std::derived_from<T, Component>;


class Entity final {
private:
    EntityId m_id = 0;
    std::vector<std::unique_ptr<Component>> m_components;
    bool m_is_in_play = false;
    bool m_is_ticking = true;

    // Only the EntitySpawner can instantiate entities
    friend class EntitySpawner;
    Entity() = default;

public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    void enter_play();
    void exit_play();
    void tick(float delta_time);
    void physics_tick(float fixed_delta_time);
    void render_tick(float delta_time);

    EntityId get_id() const;
    void set_id(EntityId id);

    bool is_in_play() const;

    bool is_ticking() const;
    void set_is_ticking(bool is_ticking);

    template<ComponentType T>
    T* add_component();

    template<ComponentType T>
    T* get_component();
};

template<ComponentType T>
T* Entity::add_component() {
    assert(get_component<T>() == nullptr && "Component of this type already exists");

    std::unique_ptr<T> component = std::make_unique<T>();
    component->set_entity(this);

    if (is_in_play()) {
        component->enter_play();
    }

    T* component_ptr = component.get();

    m_components.push_back(std::move(component));
    std::sort(m_components.begin(), m_components.end(),
              [](const auto& a, const auto& b) {
                  return static_cast<int>(a->get_priority()) < static_cast<int>(b->get_priority());
              });

    return component_ptr;
}

template<ComponentType T>
T* Entity::get_component() {
    for (auto& c : m_components) {
        if (T* casted = dynamic_cast<T*>(c.get())) {
            return casted;
        }
    }

    return nullptr;
}


#endif //CPP_PLATFORMER_ENTITY_H
