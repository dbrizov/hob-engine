#include "EntitySpawner.h"

#include <cassert>

#include "Entity.h"

void EntitySpawner::set_app(App* app) {
    m_app = app;
}

Entity* EntitySpawner::spawn_entity() {
    std::unique_ptr<Entity> entity = std::unique_ptr<Entity>(new Entity());
    EntityId entity_id = m_next_entity_id;
    m_next_entity_id += 1;
    entity->set_id(entity_id);
    entity->set_app(m_app);

    Entity* entity_ptr = entity.get();
    m_entity_spawn_requests.push_back(std::move(entity));

    return entity_ptr;
}

void EntitySpawner::destroy_entity(EntityId id) {
    m_entity_destroy_requests.insert(id);
}

Entity* EntitySpawner::get_entity(EntityId id) const {
    auto it = m_entity_index_by_id.find(id);
    if (it != m_entity_index_by_id.end()) {
        int index = it->second;
        return m_entities[index].get();
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
        entity->enter_play();
        m_entity_index_by_id[entity->get_id()] = m_entities.size();
        m_entities.emplace_back(std::move(entity));
    }
}

void EntitySpawner::resolve_destroy_requests() {
    // Swap because someone might make a destroy request in exit_play()
    std::unordered_set<EntityId> destroy_requests;
    destroy_requests.swap(m_entity_destroy_requests);

    // exit_play() while entities are still present in the map
    for (EntityId id : destroy_requests) {
        Entity* entity = get_entity(id);
        if (entity != nullptr) {
            entity->exit_play();
        }
    }

    // Erase the entities
    for (EntityId id : destroy_requests) {
        auto it = m_entity_index_by_id.find(id);
        if (it == m_entity_index_by_id.end()) {
            continue;
        }

        int index = it->second;
        int last_index = m_entities.size() - 1;

        if (index != last_index) {
            m_entities[index] = std::move(m_entities[last_index]); // move last into hole
            m_entity_index_by_id[m_entities[index]->get_id()] = index; // fix moved id's index
        }

        m_entities.pop_back();
        m_entity_index_by_id.erase(it);
    }
}
