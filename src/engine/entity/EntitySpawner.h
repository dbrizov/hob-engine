#ifndef CPP_PLATFORMER_ENTITYSPAWNER_H
#define CPP_PLATFORMER_ENTITYSPAWNER_H
#include <memory>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Entity.h"


class App;


class EntitySpawner {
    App* m_app = nullptr;

    EntityId m_next_entity_id = 0;
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::unordered_map<EntityId, int> m_entity_index_by_id;

    std::vector<std::unique_ptr<Entity>> m_entity_spawn_requests;
    std::unordered_set<EntityId> m_entity_destroy_requests;

    // App is a friend of EntitySpawner.
    // Only the App should be able to set its address at initialization.
    friend class App;
    void set_app(App* app);

public:
    Entity* spawn_entity();
    void destroy_entity(EntityId id);

    Entity* get_entity(EntityId id) const;

    auto get_entities() const {
        return m_entities | std::ranges::views::transform([](const std::unique_ptr<Entity>& ptr) {
            return ptr.get();
        });
    }

    void resolve_requests();

private:
    void resolve_spawn_requests();
    void resolve_destroy_requests();
};


#endif //CPP_PLATFORMER_ENTITYSPAWNER_H
