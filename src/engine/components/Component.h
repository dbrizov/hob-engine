#ifndef CPP_PLATFORMER_COMPONENT_H
#define CPP_PLATFORMER_COMPONENT_H


class Entity;


enum class ComponentPriority {
    INPUT = -150,
    TRANSFORM = -100,
    DEFAULT = 0,
    RENDER = 100
};


class Component {
private:
    Entity* m_entity = nullptr;

protected:
    Component() = default; // Prevent the base Component from being instantiated on its own

public:
    virtual ~Component() = default;

    Entity* get_entity() const;
    void set_entity(Entity* entity);

    virtual ComponentPriority priority() const;

    virtual void enter_play();
    virtual void exit_play();
    virtual void tick(float delta_time);
    virtual void physics_tick(float delta_time);
    virtual void render_tick(float delta_time);
};


#endif //CPP_PLATFORMER_COMPONENT_H
