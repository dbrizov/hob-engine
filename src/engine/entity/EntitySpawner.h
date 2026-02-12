#ifndef CPP_PLATFORMER_ENTITYSPAWNER_H
#define CPP_PLATFORMER_ENTITYSPAWNER_H
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Entity.h"


class App;


class EntitySpawner {
    App& m_app;
    EntityId m_camera_entity_id = INVALID_ENTITY_ID;

    EntityId m_next_entity_id = 0;
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::unordered_map<EntityId, size_t> m_entity_index_by_id;

    std::vector<std::unique_ptr<Entity>> m_entity_spawn_requests;
    std::unordered_set<EntityId> m_entity_destroy_requests;

    // App is a friend of EntitySpawner so that:
    // - It can initialize a camera entity.
    // - Resolve spawn requests via resolve_requests().
    friend class App;

public:
    explicit EntitySpawner(App& app);

    Entity& spawn_entity();
    void destroy_entity(EntityId id);

    Entity* get_entity(EntityId id) const;
    void get_entities(std::vector<Entity*>& out_entities) const;
    void get_ticking_entities(std::vector<Entity*>& out_entities) const;

    Entity* get_camera_entity() const;

private:
    void spawn_camera_entity(uint32_t logical_resolution_width, uint32_t logical_resolution_height);

    void resolve_requests();
    void resolve_spawn_requests();
    void resolve_destroy_requests();
};


#endif //CPP_PLATFORMER_ENTITYSPAWNER_H
