#include "EntitySpawner.h"

#include <cassert>

#include "Entity.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/ImageComponent.h"
#include "engine/components/physics/RigidbodyComponent.h"
#include "engine/components/TransformComponent.h"
#include "engine/core/App.h"


namespace hob {
    EntitySpawner::EntitySpawner(App& app)
        : m_app(app) {
        const GraphicsConfig& graphics_config = app.get_config().graphics_config;
        spawn_camera_entity(graphics_config.logical_resolution_width, graphics_config.logical_resolution_height);
    }

    EntitySpawner::~EntitySpawner() {
        for (auto& entity : m_entities) {
            entity->exit_play();
        }
    }

    Entity& EntitySpawner::spawn_entity() {
        std::unique_ptr<Entity> entity = std::unique_ptr<Entity>(new Entity(m_app));

        EntityId entity_id = m_next_entity_id;
        m_next_entity_id += 1;
        entity->set_id(entity_id);
        entity->add_component<TransformComponent>();

        m_entity_spawn_requests.push_back(std::move(entity));

        return *m_entity_spawn_requests.back();
    }

    void EntitySpawner::destroy_entity(EntityId id) {
        // Remove from pending spawns if present
        std::erase_if(m_entity_spawn_requests, [&](const std::unique_ptr<Entity>& e) {
            return e->get_id() == id;
        });

        // Mark for destroy if already in play
        m_entity_destroy_requests.insert(id);
    }

    Entity* EntitySpawner::get_entity(EntityId id) const {
        auto it = m_entity_index_by_id.find(id);
        if (it != m_entity_index_by_id.end()) {
            size_t index = it->second;
            assert(index < m_entities.size() && "Index out of range");
            return m_entities[index].get();
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

        std::sort(out_entities.begin(), out_entities.end(),
                  [](const auto& a, const auto& b) {
                      return a->get_priority() < b->get_priority();
                  });
    }

    void EntitySpawner::get_physics_entities(std::vector<Entity*>& out_entities) const {
        out_entities.clear();
        out_entities.reserve(m_entities.size());
        for (const auto& entity : m_entities) {
            if (!entity->is_ticking()) {
                continue;
            }

            const RigidbodyComponent* rigidbody = entity->get_rigidbody();
            if (rigidbody == nullptr || !rigidbody->has_body() || rigidbody->get_body_type() == BodyType::STATIC) {
                continue;
            }

            out_entities.push_back(entity.get());
        }

        std::sort(out_entities.begin(), out_entities.end(),
                  [](const auto& a, const auto& b) {
                      return a->get_priority() < b->get_priority();
                  });
    }

    void EntitySpawner::get_renderable_entities(std::vector<const Entity*>& out_entities) const {
        out_entities.clear();
        out_entities.reserve(m_entities.size());
        for (const auto& entity : m_entities) {
            const ImageComponent* img_comp = entity->get_component<ImageComponent>();
            if (img_comp == nullptr) {
                continue;
            }

            out_entities.push_back(entity.get());
        }

        std::sort(out_entities.begin(), out_entities.end(),
                  [](const auto& a, const auto& b) {
                      return a->get_priority() < b->get_priority();
                  });
    }

    Entity* EntitySpawner::get_camera_entity() const {
        Entity* camera_entity = get_entity(m_camera_entity_id);
        assert(camera_entity != nullptr && "There is no camera");

        return camera_entity;
    }

    void EntitySpawner::spawn_camera_entity(uint32_t logical_resolution_width, uint32_t logical_resolution_height) {
        Entity& camera_entity = spawn_entity();
        camera_entity.set_priority(entity_priority::EP_CAMERA);

        CameraComponent* camera_component = camera_entity.add_component<CameraComponent>();
        camera_component->init(logical_resolution_width, logical_resolution_height);

        m_camera_entity_id = camera_entity.get_id();
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
            m_entity_index_by_id[entity_id] = m_entities.size();
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
            auto it = m_entity_index_by_id.find(id);
            if (it == m_entity_index_by_id.end()) {
                continue;
            }

            size_t index = it->second;
            size_t last_index = m_entities.size() - 1;

            if (index != last_index) {
                m_entities[index] = std::move(m_entities[last_index]); // move last into hole
                m_entity_index_by_id[m_entities[index]->get_id()] = index; // fix moved id's index
            }

            m_entities.pop_back();
            m_entity_index_by_id.erase(it);
        }
    }
}
