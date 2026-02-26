#include "contact_logger_component.h"

#include "engine/components/physics/collider_component.h"
#include "engine/core/logging.h"
#include "engine/entity/entity.h"

namespace game {
    ContactLoggerComponent::ContactLoggerComponent(hob::Entity& entity)
        : Component(entity) {
    }

    void ContactLoggerComponent::on_collision_enter(const hob::ColliderComponent* other_collider) {
        hob::debug::log_error("collision_enter: {}", other_collider->get_entity().get_id());
    }

    void ContactLoggerComponent::on_collision_exit(const hob::ColliderComponent* other_collider) {
        hob::debug::log_error("collision_exit: {}", other_collider->get_entity().get_id());
    }

    void ContactLoggerComponent::on_trigger_enter(const hob::ColliderComponent* other_collider) {
        hob::debug::log_error("trigger_enter: {}", other_collider->get_entity().get_id());
    }

    void ContactLoggerComponent::on_trigger_exit(const hob::ColliderComponent* other_collider) {
        hob::debug::log_error("trigger_exit: {}", other_collider->get_entity().get_id());
    }
}
