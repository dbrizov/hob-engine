#ifndef CPP_PLATFORMER_ENTITY_H
#define CPP_PLATFORMER_ENTITY_H
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "engine/components/Component.h"


class App;

using EntityId = int;

template<typename T>
concept ComponentType = std::derived_from<T, Component>;


class Entity final {
    App* m_app = nullptr;
    EntityId m_id = 0;
    bool m_is_in_play = false;
    bool m_is_ticking = false;
    std::vector<std::unique_ptr<Component>> m_components;

    // EntitySpawner is a friend of Entity for 2 reasons.
    // 1. Only the EntitySpawner can create entities.
    // 2. Only the EntitySpawner should be able to set the App address when an entity is spawned.
    friend class EntitySpawner;
    Entity() = default;
    void set_app(App* app);

public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    void enter_play();
    void exit_play();
    void tick(float delta_time);
    void physics_tick(float fixed_delta_time);
    void render_tick(float delta_time, RenderQueue& render_queue);

    App* get_app() const;

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
