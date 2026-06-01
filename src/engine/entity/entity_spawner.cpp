#include "entity_spawner.h"

#include "entity.h"
#include "engine/components/physics/rigidbody_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"

namespace hob {
    EntitySpawner::EntitySpawner(Engine& engine)
        : m_engine(engine) {
    }

    EntitySpawner::~EntitySpawner() {
        clear();
    }

    Entity& EntitySpawner::spawn_entity() {
        std::unique_ptr<Entity> entity = std::unique_ptr<Entity>(new Entity(m_engine));

        EntityId entity_id = m_next_entity_id;
        m_next_entity_id += 1;
        entity->set_id(entity_id);
        entity->add_component<TransformComponent>();

        Entity* entity_ptr = entity.get();
        m_entity_spawn_requests.push_back(std::move(entity));
        m_entity_records[entity_id] = EntityRecord{entity_ptr, INVALID_ENTITY_INDEX};

        return *entity_ptr;
    }

    void EntitySpawner::destroy_entity(EntityId id) {
        // Remove from pending spawns if present.
        // The unique_ptr is freed here, so also drop the record to avoid leaving a dangling pointer behind.
        const size_t erased_count = std::erase_if(m_entity_spawn_requests, [&](const std::unique_ptr<Entity>& e) {
            return e->get_id() == id;
        });

        const bool was_pending = erased_count > 0;
        if (was_pending) {
            m_entity_records.erase(id);
            return;
        }

        // Mark for destroy if already in play
        m_entity_destroy_requests.insert(id);
    }

    Entity* EntitySpawner::get_entity(EntityId id) const {
        auto it = m_entity_records.find(id);
        if (it != m_entity_records.end()) {
            return it->second.ptr;
        }

        return nullptr;
    }

    void EntitySpawner::get_entities(std::vector<Entity*>& out_entities) const {
        out_entities.clear();
        out_entities.reserve(m_entities.size());
        for (const auto& entity : m_entities) {
            out_entities.push_back(entity.get());
        }
    }

    void EntitySpawner::get_ticking_entities(std::vector<Entity*>& out_entities) const {
        out_entities.clear();
        out_entities.reserve(m_entities.size());
        for (const auto& entity : m_entities) {
            if (entity->is_ticking()) {
                out_entities.push_back(entity.get());
            }
        }
    }

    void EntitySpawner::get_physics_entities(std::vector<Entity*>& out_entities) const {
        out_entities.clear();
        out_entities.reserve(m_entities.size());
        for (const auto& entity : m_entities) {
            if (!entity->is_ticking()) {
                continue;
            }

            const RigidbodyComponent* rigidbody = entity->get_rigidbody();
            if (rigidbody == nullptr || !rigidbody->has_body() || rigidbody->get_body_type() == BodyType::Static) {
                continue;
            }

            out_entities.push_back(entity.get());
        }
    }

    void EntitySpawner::get_renderable_entities(std::vector<const Entity*>& out_entities) const {
        out_entities.clear();
        out_entities.reserve(m_entities.size());
        for (const auto& entity : m_entities) {
            const SpriteComponent* sprite_comp = entity->get_component<SpriteComponent>();
            if (sprite_comp == nullptr) {
                continue;
            }

            out_entities.push_back(entity.get());
        }
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
            m_entity_records[entity_id].live_index = m_entities.size();
            m_entities.emplace_back(std::move(entity));
            m_entities.back()->enter_play();
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
            auto it = m_entity_records.find(id);
            if (it == m_entity_records.end()) {
                continue;
            }

            size_t index = it->second.live_index;
            if (index != INVALID_ENTITY_INDEX) {
                size_t last_index = m_entities.size() - 1;
                if (index != last_index) {
                    m_entities[index] = std::move(m_entities[last_index]); // move last into hole
                    m_entity_records[m_entities[index]->get_id()].live_index = index; // fix record's index
                }

                m_entities.pop_back();
            }

            m_entity_records.erase(it);
        }
    }

    void EntitySpawner::clear() {
        for (auto& entity : m_entities) {
            entity->exit_play();
        }

        m_entities.clear();
        m_entity_records.clear();
        m_entity_spawn_requests.clear();
        m_entity_destroy_requests.clear();
    }
}
