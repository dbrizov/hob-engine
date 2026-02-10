#ifndef CPP_PLATFORMER_COMPONENT_H
#define CPP_PLATFORMER_COMPONENT_H


class App;
class RenderQueue;
class Entity;


enum class ComponentPriority {
    INPUT = -200,
    TRANSFORM = -100,
    DEFAULT = 0,
    RENDER = 100
};


class Component {
    Entity* m_entity = nullptr;

protected:
    Component() = default; // Prevent the base Component from being instantiated on its own

public:
    virtual ~Component() = default;

    App* get_app() const;

    Entity* get_entity() const;
    void set_entity(Entity* entity);

    virtual ComponentPriority get_priority() const;

    virtual void enter_play();
    virtual void exit_play();
    virtual void tick(float delta_time);
    virtual void physics_tick(float fixed_delta_time);
    virtual void render_tick(float delta_time, RenderQueue& render_queue);
};


#endif //CPP_PLATFORMER_COMPONENT_H
