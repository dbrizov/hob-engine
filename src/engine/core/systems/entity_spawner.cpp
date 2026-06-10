#include "entity_spawner.h"

#include "engine/entity/entity.h"
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

        const EntityId entity_id = m_next_entity_id;
        m_next_entity_id += 1;
        entity->set_id(entity_id);
        entity->add_component<TransformComponent>();

        Entity* entity_ptr = entity.get();
        m_entity_spawn_requests.push_back(std::move(entity));
        m_entity_records[entity_id] = EntityRecord{entity_ptr, INVALID_ENTITY_INDEX};

        return *entity_ptr;
    }

    void EntitySpawner::destroy_entity(EntityId id) {
        auto record_it = m_entity_records.find(id);
        if (record_it == m_entity_records.end()) {
            return; // already destroyed or never existed
        }

        Entity* entity = record_it->second.ptr;
        const bool was_pending = record_it->second.live_index == INVALID_ENTITY_INDEX;

        // Destroy the whole subtree. Snapshot the children because each recursive call mutates this list.
        TransformComponent* transform = entity->get_transform();
        const std::vector<TransformComponent*> children = transform->get_children();
        for (const TransformComponent* child : children) {
            destroy_entity(child->get_entity().get_id());
        }

        if (was_pending) {
            // Pending entities never enter play, so exit_play()/detach won't run.
            // Detach synchronously to avoid leaving a dangling pointer in a surviving parent (or in a still-in-play child).
            transform->detach_from_hierarchy();

            // Drop the pending spawn request, which frees the Entity's unique_ptr.
            std::erase_if(m_entity_spawn_requests, [&](const std::unique_ptr<Entity>& e) {
                return e->get_id() == id;
            });

            // The record's raw Entity* now dangles, so erase it too.
            m_entity_records.erase(id);
            return;
        }

        // Mark for destroy if already in play. In-play subtree members detach in exit_play() during resolve.
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
            const EntityId entity_id = entity->get_id();
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
        for (const EntityId id : destroy_requests) {
            Entity* entity = get_entity(id);
            if (entity != nullptr) {
                entity->exit_play();
            }
        }

        // Erase the entities
        for (const EntityId id : destroy_requests) {
            auto it = m_entity_records.find(id);
            if (it == m_entity_records.end()) {
                continue;
            }

            const size_t index = it->second.live_index;
            if (index != INVALID_ENTITY_INDEX) {
                const size_t last_index = m_entities.size() - 1;
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
