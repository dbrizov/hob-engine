#ifndef HOB_ENGINE_CONTACTLOGGERCOMPONENT_H
#define HOB_ENGINE_CONTACTLOGGERCOMPONENT_H
#include "engine/components/Component.h"


class ContactLoggerComponent : public Component {
public:
    explicit ContactLoggerComponent(Entity& entity);

    virtual void on_collision_enter(const ColliderComponent* other_collider) override;
    virtual void on_collision_exit(const ColliderComponent* other_collider) override;
    virtual void on_trigger_enter(const ColliderComponent* other_collider) override;
    virtual void on_trigger_exit(const ColliderComponent* other_collider) override;
};


#endif //HOB_ENGINE_CONTACTLOGGERCOMPONENT_H