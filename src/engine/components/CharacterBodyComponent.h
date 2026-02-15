#ifndef HOB_ENGINE_CHARACTERBODYCOMPONENT_H
#define HOB_ENGINE_CHARACTERBODYCOMPONENT_H
#include "Component.h"
#include "engine/math/Vector2.h"


class BoxColliderComponent;
class RigidbodyComponent;


class CharacterBodyComponent : public Component {
    RigidbodyComponent* m_rigidbody = nullptr;
    BoxColliderComponent* m_box_collider = nullptr;

public:
    explicit CharacterBodyComponent(Entity& entity);

    virtual int get_priority() const override;

    Vector2 get_velocity() const;
    void set_velocity(const Vector2& velocity);
};


#endif //HOB_ENGINE_CHARACTERBODYCOMPONENT_H
