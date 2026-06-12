#pragma once

#include <limits>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "engine/entity/entity.h"

namespace hob {
    class Engine;
    class SpriteComponent;

    using EntityIndex = uint32_t;
    constexpr EntityIndex INVALID_ENTITY_INDEX = std::numeric_limits<EntityIndex>::max();

    // Per-id bookkeeping.
    // - 'ptr' is valid from spawn_entity() until the entity exits play.
    // - 'live_index' is the slot in m_entities once the entity enters play.
    //   While the entity is still in m_entity_spawn_requests 'live_index' stays INVALID_ENTITY_INDEX.
    struct EntityRecord {
        Entity* ptr = nullptr;
        EntityIndex live_index = INVALID_ENTITY_INDEX;
    };

    class EntitySpawner {
        Engine& m_engine;

        EntityId m_next_entity_id = 0;
        std::vector<std::unique_ptr<Entity>> m_entities;
        std::unordered_map<EntityId, EntityRecord> m_entity_records;

        std::vector<std::unique_ptr<Entity>> m_entity_spawn_requests;
        std::unordered_set<EntityId> m_entity_destroy_requests;

        // Registry of in-play sprites, maintained incrementally by SpriteComponent's enter_play/exit_play.
        std::vector<SpriteComponent*> m_renderables;

        // Engine is a friend of EntitySpawner so that:
        // - It can handle the lifecycle of entities
        friend class Engine;

    public:
        explicit EntitySpawner(Engine& engine);
        ~EntitySpawner();

        Entity& spawn_entity();
        void destroy_entity(EntityId id);

        Entity* get_entity(EntityId id) const;
        void get_entities(std::vector<Entity*>& out_entities) const;
        void get_ticking_entities(std::vector<Entity*>& out_entities) const;
        void get_physics_entities(std::vector<Entity*>& out_entities) const;

        void register_renderable(SpriteComponent* sprite);
        void unregister_renderable(SpriteComponent* sprite);
        const std::vector<SpriteComponent*>& get_renderables() const;

    private:
        void resolve_requests();
        void resolve_spawn_requests();
        void resolve_destroy_requests();

        // Destroy every live and pending entity, calling exit_play on those in play.
        // Call this before Engine's other subsystems start tearing down so that
        // components which hold references into other subsystems (e.g. LuaScriptComponent
        // owns a sol::table tied to the Lua state) are destroyed while those subsystems are still alive.
        void clear();
    };
}
