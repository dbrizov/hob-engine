#ifndef HOB_ENGINE_TRANSFORMCOMPONENT_H
#define HOB_ENGINE_TRANSFORMCOMPONENT_H
#include "Component.h"
#include "engine/math/Vector2.h"


class TransformComponent : public Component {
    Vector2 m_position;
    float m_rotation = 0.0f; // In degrees
    Vector2 m_scale = Vector2(1.0f, 1.0f);

    // Physics is a friend class of TransformComponent so that
    // it can set the m_prev_position that is used for Physics interpolation
    friend class Physics;
    Vector2 m_prev_position;

public:
    explicit TransformComponent(Entity& entity);

    virtual int get_priority() const override;

    Vector2 get_position() const;
    void set_position(const Vector2& position);

    float get_rotation() const;
    void set_rotation(float rotation);

    Vector2 get_scale() const;
    void set_scale(const Vector2& scale);

    Vector2 get_prev_position() const;

private:
    void set_prev_position(const Vector2& prev_position);
};


#endif //HOB_ENGINE_TRANSFORMCOMPONENT_H
