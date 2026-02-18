#include "ContactLoggerComponent.h"

#include <fmt/base.h>

#include "engine/components/physics/ColliderComponent.h"
#include "engine/entity/Entity.h"

ContactLoggerComponent::ContactLoggerComponent(Entity& entity)
    : Component(entity) {
}

void ContactLoggerComponent::on_collision_enter(const ColliderComponent* other_collider) {
    fmt::println(stderr, "collision_enter: {}", other_collider->get_entity().get_id());
}

void ContactLoggerComponent::on_collision_exit(const ColliderComponent* other_collider) {
    fmt::println(stderr, "collision_exit: {}", other_collider->get_entity().get_id());
}

void ContactLoggerComponent::on_trigger_enter(const ColliderComponent* other_collider) {
    fmt::println(stderr, "trigger_enter: {}", other_collider->get_entity().get_id());
}

void ContactLoggerComponent::on_trigger_exit(const ColliderComponent* other_collider) {
    fmt::println(stderr, "trigger_exit: {}", other_collider->get_entity().get_id());
}
