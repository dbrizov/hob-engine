#pragma once

#include "engine/components/component.h"

namespace game {
    class ContactLoggerComponent : public hob::Component {
    public:
        explicit ContactLoggerComponent(hob::Entity& entity);

        virtual void on_collision_enter(const hob::ColliderComponent* other_collider) override;
        virtual void on_collision_exit(const hob::ColliderComponent* other_collider) override;
        virtual void on_trigger_enter(const hob::ColliderComponent* other_collider) override;
        virtual void on_trigger_exit(const hob::ColliderComponent* other_collider) override;
    };
}
