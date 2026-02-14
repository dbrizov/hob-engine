#ifndef HOB_ENGINE_ENTITY_H
#define HOB_ENGINE_ENTITY_H
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "engine/components/Component.h"


class RigidbodyComponent;
class TransformComponent;
class App;

using EntityId = int;
constexpr EntityId INVALID_ENTITY_ID = -1;

template<typename T>
concept ComponentType = std::derived_from<T, Component>;


namespace entity_priority {
    constexpr int EP_CAMERA = -1000;
    constexpr int EP_DEFAULT = 0;
}


class Entity final {
    App& m_app;
    EntityId m_id = 0;
    int m_priority = entity_priority::EP_DEFAULT;
    bool m_is_in_play = false;
    bool m_is_ticking = false;
    std::vector<std::unique_ptr<Component>> m_components;
    mutable TransformComponent* m_transform = nullptr;
    mutable RigidbodyComponent* m_rigidbody = nullptr;

    // EntitySpawner is a friend of Entity.
    // - Only the EntitySpawner can create entities.
    friend class EntitySpawner;
    explicit Entity(App& app);

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

    App& get_app() const;

    EntityId get_id() const;
    void set_id(EntityId id);

    int get_priority() const;
    void set_priority(int priority);

    bool is_in_play() const;

    bool is_ticking() const;
    void set_is_ticking(bool is_ticking);

    TransformComponent* get_transform() const;
    RigidbodyComponent* get_rigidbody() const;

    template<ComponentType T>
    T* add_component();

    template<ComponentType T>
    T* get_component() const;
};

template<ComponentType T>
T* Entity::add_component() {
    assert(get_component<T>() == nullptr && "Component of this type already exists");

    std::unique_ptr<T> component = std::make_unique<T>(*this);

    if (is_in_play()) {
        component->enter_play();
    }

    T* component_ptr = component.get();

    m_components.push_back(std::move(component));
    std::sort(m_components.begin(), m_components.end(),
              [](const auto& a, const auto& b) {
                  return a->get_priority() < b->get_priority();
              });

    return component_ptr;
}

template<ComponentType T>
T* Entity::get_component() const {
    for (auto& c : m_components) {
        if (T* casted = dynamic_cast<T*>(c.get())) {
            return casted;
        }
    }

    return nullptr;
}


#endif //HOB_ENGINE_ENTITY_H
