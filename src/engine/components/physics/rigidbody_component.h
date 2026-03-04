#pragma once

#include <box2d/id.h>

#include "engine/components/component.h"

namespace hob {
    enum class BodyType {
        Static,
        Dynamic,
        Kinematic
    };

    class RigidbodyComponent : public Component {
        b2BodyId m_body_id = b2_nullBodyId;
        BodyType m_body_type = BodyType::Static;
        bool m_has_fixed_rotation = false;

    public:
        explicit RigidbodyComponent(Entity& entity);

        virtual void enter_play() override;
        virtual void exit_play() override;

        b2BodyId get_body_id() const;
        bool has_body() const;

        BodyType get_body_type() const;
        void set_body_type(BodyType body_type);

        bool has_fixed_rotation() const;
        void set_fixed_rotation(bool has_fixed_rotation);
    };
}
