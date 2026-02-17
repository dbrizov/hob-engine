#ifndef HOB_ENGINE_COMPONENT_H
#define HOB_ENGINE_COMPONENT_H


class App;
class RenderQueue;
class Entity;


namespace component_priority {
    constexpr int CP_INPUT = -200;
    constexpr int CP_TRANSFORM = -100;
    constexpr int CP_DEFAULT = 0;
    constexpr int CP_RENDER = 100;
}


class Component {
    Entity& m_entity;

protected:
    explicit Component(Entity& entity); // prevent the base Component from being instantiated on its own

public:
    virtual ~Component() = default;

    App& get_app() const;
    Entity& get_entity() const;

    virtual int get_priority() const;

    virtual void enter_play();
    virtual void exit_play();
    virtual void tick(float delta_time);
    virtual void physics_tick(float fixed_delta_time);
    virtual void render_tick(float delta_time, RenderQueue& render_queue);
};


#endif //HOB_ENGINE_COMPONENT_H
