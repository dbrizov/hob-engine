#ifndef CPP_PLATFORMER_ENTITY_H
#define CPP_PLATFORMER_ENTITY_H
#include <cstdint>


using EntityId = uint32_t;


class Entity {
private:
    friend class EntitySpawner;

    Entity() = default;

    EntityId m_id = 0;
    // TODO  m_components
    bool m_is_in_play = false;
    bool m_is_ticking = true;

public:
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    void enter_play();
    void exit_play();
    void tick(float delta_time);
    void physics_tick(float fixed_delta_time);
    void render_tick(float delta_time);

    EntityId id() const;
    void set_id(EntityId id);

    bool is_in_play() const;

    bool is_ticking() const;
    void set_is_ticking(bool is_ticking);

    // TODO manage components
};


#endif //CPP_PLATFORMER_ENTITY_H
