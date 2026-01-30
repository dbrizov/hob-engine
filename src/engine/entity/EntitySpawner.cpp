#include "EntitySpawner.h"

#include <cassert>

EntitySpawner::EntityRange EntitySpawner::get_entities() {
    return EntityRange(m_entities);
}

EntitySpawner::ConstEntityRange EntitySpawner::get_entities_const() const {
    return ConstEntityRange(m_entities);
}

Entity* EntitySpawner::spawn_entity() {
    std::unique_ptr<Entity> entity = std::unique_ptr<Entity>(new Entity());
    const EntityId entity_id = m_next_entity_id;
    entity->set_id(entity_id);
    m_next_entity_id += 1;

    Entity* entity_ptr = entity.get();
    m_entity_spawn_requests.push_back(std::move(entity));

    return entity_ptr;
}

void EntitySpawner::destroy_entity(EntityId id) {
    m_entity_destroy_requests.insert(id);
}

Entity* EntitySpawner::get_entity(EntityId id) const {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second.get();
    }

    return nullptr;
}

void EntitySpawner::resolve_requests() {
    resolve_spawn_requests();
    resolve_destroy_requests();
}

void EntitySpawner::resolve_spawn_requests() {
    // Swap because someone might make a spawn request in enter_play()
    std::vector<std::unique_ptr<Entity>> spawn_requests;
    spawn_requests.swap(m_entity_spawn_requests);

    for (auto& entity : spawn_requests) {
        EntityId entity_id = entity->get_id();
        entity->enter_play();
        auto [it, inserted] = m_entities.emplace(entity_id, std::move(entity));
        assert(inserted && "Duplicate EntityId while trying to spawn an Entity");
    }
}

void EntitySpawner::resolve_destroy_requests() {
    // Swap because someone might make a destroy request in exit_play()
    std::unordered_set<EntityId> destroy_requests;
    destroy_requests.swap(m_entity_destroy_requests);

    // exit_play() while entities are still present in the map
    for (EntityId id : destroy_requests) {
        auto it = m_entities.find(id);
        if (it != m_entities.end()) {
            it->second->exit_play();
        }
    }

    // Erase the entities
    for (EntityId id : destroy_requests) {
        m_entities.erase(id);
    }
}
