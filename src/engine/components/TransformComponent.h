#ifndef CPP_PLATFORMER_TRANSFORMCOMPONENT_H
#define CPP_PLATFORMER_TRANSFORMCOMPONENT_H
#include "Component.h"
#include "engine/math/Vector2.h"


class TransformComponent : public Component {
private:
    Vector2 m_position;
    Vector2 m_prev_position;
    Vector2 m_scale;

public:
    virtual ComponentPriority get_priority() const override;

    Vector2 get_position() const;
    void set_position(const Vector2& position);

    Vector2 get_prev_position() const;

    Vector2 get_scale() const;
    void set_scale(const Vector2& scale);
};


#endif //CPP_PLATFORMER_TRANSFORMCOMPONENT_H
