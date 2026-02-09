#ifndef CPP_PLATFORMER_TRANSFORMCOMPONENT_H
#define CPP_PLATFORMER_TRANSFORMCOMPONENT_H
#include "Component.h"
#include "engine/math/Vector2.h"


class TransformComponent : public Component {
    Vector2 m_position;
    Vector2 m_prev_position; // Used for Physics interpolation
    Vector2 m_scale = Vector2(1.0f, 1.0f);

    // Physics is a friend class of TransformComponent so that
    // it can set the m_prev_position that is used for Physics interpolation
    friend class Physics;

public:
    virtual ComponentPriority get_priority() const override;

    Vector2 get_position() const;
    void set_position(const Vector2& position);

    Vector2 get_scale() const;
    void set_scale(const Vector2& scale);

    Vector2 get_prev_position() const;

private:
    void set_prev_position(const Vector2& prev_position);
};


#endif //CPP_PLATFORMER_TRANSFORMCOMPONENT_H
