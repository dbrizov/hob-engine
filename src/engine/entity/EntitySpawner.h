#ifndef CPP_PLATFORMER_ENTITYSPAWNER_H
#define CPP_PLATFORMER_ENTITYSPAWNER_H
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Entity.h"


class EntitySpawner {
private:
    EntityId m_next_entity_id = 0;
    std::unordered_map<EntityId, std::unique_ptr<Entity>> m_entities;
    std::vector<std::unique_ptr<Entity>> m_entity_spawn_requests;
    std::unordered_set<EntityId> m_entity_destroy_requests;

public:
    // Iterators
    struct EntityRange {
        std::unordered_map<EntityId, std::unique_ptr<Entity>>& map;

        struct It {
            std::unordered_map<EntityId, std::unique_ptr<Entity>>::iterator it;

            Entity& operator*() const { return *it->second; }
            Entity* operator->() const { return it->second.get(); }

            It& operator++() {
                ++it;
                return *this;
            }

            bool operator!=(const It& other) const { return it != other.it; }
        };

        It begin() { return {map.begin()}; }
        It end() { return {map.end()}; }
    };

    struct ConstEntityRange {
        const std::unordered_map<EntityId, std::unique_ptr<Entity>>& map;

        struct It {
            std::unordered_map<EntityId, std::unique_ptr<Entity>>::const_iterator it;

            const Entity& operator*() const { return *it->second; }
            const Entity* operator->() const { return it->second.get(); }

            It& operator++() {
                ++it;
                return *this;
            }

            bool operator!=(const It& other) const { return it != other.it; }
        };

        It begin() const { return {map.begin()}; }
        It end() const { return {map.end()}; }
    };

public:
    EntityRange get_entities();
    ConstEntityRange get_entities_const() const;

    EntityId spawn_entity();
    void destroy_entity(EntityId id);

    void resolve_requests();

private:
    void resolve_spawn_requests();
    void resolve_destroy_requests();
};


#endif //CPP_PLATFORMER_ENTITYSPAWNER_H
