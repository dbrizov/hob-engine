#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_component_schema.h"
#include "lua_meta.h"
#include "lua_type_names.h" // IWYU pragma: keep

#include <string>

#include "engine/components/camera_component.h"
#include "engine/components/input_component.h"
#include "engine/components/physics/box_collider_component.h"
#include "engine/components/physics/capsule_collider_component.h"
#include "engine/components/physics/character_body_component.h"
#include "engine/components/physics/circle_collider_component.h"
#include "engine/components/physics/collider_component.h"
#include "engine/components/physics/rigidbody_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/debug.h"
#include "engine/core/engine.h"
#include "engine/core/logging.h"
#include "engine/core/systems/input.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    void LuaScriptSystem::bind_components() {
        sol::state& m_lua = m_impl->lua;
        LuaMetaRegistry& m_meta = m_impl->meta;
        LuaComponentSchemaRegistry& m_schemas = m_impl->component_schemas;
        const EntitySpawner& m_spawner = m_engine.get_entity_spawner();

        bind_usertype<Component>(m_lua, m_meta, "Component")
            .method("get_entity", [](Component& c) { return EntityHandle(c.get_entity().get_id()); })
            .op_tostring(&Component::to_string);

        bind_usertype<TransformComponent>(m_lua, m_meta, "TransformComponent", Bases<Component>{})
            .method("get_position", &TransformComponent::get_position)
            .method("set_position", &TransformComponent::set_position, {"position"})
            .method("get_rotation", &TransformComponent::get_rotation)
            .method("set_rotation", &TransformComponent::set_rotation, {"radians"})
            .method("get_scale", &TransformComponent::get_scale)
            .method("set_scale", &TransformComponent::set_scale, {"scale"});

        bind_usertype<SpriteComponent>(m_lua, m_meta, "SpriteComponent", Bases<Component>{})
            .method("has_texture", &SpriteComponent::has_texture)
            .method("set_texture", &SpriteComponent::set_texture, {"relative_path"})
            .method("clear_texture", &SpriteComponent::clear_texture)
            .method("get_pivot", &SpriteComponent::get_pivot)
            .method("set_pivot", &SpriteComponent::set_pivot, {"pivot"})
            .method("get_scale", &SpriteComponent::get_scale)
            .method("set_scale", &SpriteComponent::set_scale, {"scale"})
            .method("get_tint", &SpriteComponent::get_tint)
            .method("set_tint", &SpriteComponent::set_tint, {"color"})
            .method("get_z_index", &SpriteComponent::get_z_index)
            .method("set_z_index", &SpriteComponent::set_z_index, {"z_index"})
            .method("get_pixels_per_meter", &SpriteComponent::get_pixels_per_meter)
            .method("set_pixels_per_meter", &SpriteComponent::set_pixels_per_meter, {"value"});

        bind_usertype<CameraComponent>(m_lua, m_meta, "CameraComponent", Bases<Component>{})
            .method("world_to_screen",
                    sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::world_to_screen),
                    {"world_pos"})
            .method("screen_to_world",
                    sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::screen_to_world),
                    {"screen_pos"});

        bind_enum<BodyType>(m_lua, m_meta, "BodyType", {
                                {"Static", BodyType::Static},
                                {"Dynamic", BodyType::Dynamic},
                                {"Kinematic", BodyType::Kinematic},
                            });

        bind_usertype<RigidbodyComponent>(m_lua, m_meta, "RigidbodyComponent", Bases<Component>{})
            .method("get_body_type", &RigidbodyComponent::get_body_type)
            .method("set_body_type", &RigidbodyComponent::set_body_type, {"body_type"})
            .method("has_fixed_rotation", &RigidbodyComponent::has_fixed_rotation)
            .method("set_fixed_rotation", &RigidbodyComponent::set_fixed_rotation, {"fixed"})
            .method("get_velocity", &RigidbodyComponent::get_velocity)
            .method("set_velocity", &RigidbodyComponent::set_velocity, {"velocity"})
            .method("get_position", &RigidbodyComponent::get_position)
            .method("set_position", &RigidbodyComponent::set_position, {"position"})
            .method("get_rotation", &RigidbodyComponent::get_rotation)
            .method("set_rotation", &RigidbodyComponent::set_rotation, {"radians"});

        bind_usertype<ColliderComponent>(m_lua, m_meta, "ColliderComponent", Bases<Component>{})
            .method("get_density", &ColliderComponent::get_density)
            .method("set_density", &ColliderComponent::set_density, {"density"})
            .method("get_friction", &ColliderComponent::get_friction)
            .method("set_friction", &ColliderComponent::set_friction, {"friction"})
            .method("get_bounciness", &ColliderComponent::get_bounciness)
            .method("set_bounciness", &ColliderComponent::set_bounciness, {"bounciness"})
            .method("get_collision_layer", &ColliderComponent::get_collision_layer)
            .method("set_collision_layer", &ColliderComponent::set_collision_layer, {"layer"})
            .method("get_collision_mask", &ColliderComponent::get_collision_mask)
            .method("set_collision_mask", &ColliderComponent::set_collision_mask, {"mask"})
            .method("is_trigger", &ColliderComponent::is_trigger)
            .method("set_trigger", &ColliderComponent::set_trigger, {"trigger"})
            .method("get_baked_scale", &ColliderComponent::get_baked_scale)
            .method("on_scale_changed", &ColliderComponent::on_scale_changed);

        bind_usertype<BoxColliderComponent>(m_lua, m_meta, "BoxColliderComponent",
                                            Bases<ColliderComponent, Component>{})
            .method("get_aabb", &BoxColliderComponent::get_aabb)
            .method("set_aabb", &BoxColliderComponent::set_aabb, {"aabb"})
            .method("get_scaled_aabb", &BoxColliderComponent::get_scaled_aabb);

        bind_usertype<CapsuleColliderComponent>(m_lua, m_meta, "CapsuleColliderComponent",
                                                Bases<ColliderComponent, Component>{})
            .method("get_capsule", &CapsuleColliderComponent::get_capsule)
            .method("set_capsule", &CapsuleColliderComponent::set_capsule, {"capsule"})
            .method("get_scaled_capsule", &CapsuleColliderComponent::get_scaled_capsule);

        bind_usertype<CircleColliderComponent>(m_lua, m_meta, "CircleColliderComponent",
                                               Bases<ColliderComponent, Component>{})
            .method("get_circle", &CircleColliderComponent::get_circle)
            .method("set_circle", &CircleColliderComponent::set_circle, {"circle"})
            .method("get_scaled_circle", &CircleColliderComponent::get_scaled_circle);

        bind_usertype<CharacterBodyComponent>(m_lua, m_meta, "CharacterBodyComponent", Bases<Component>{})
            .method("get_collision_layer", &CharacterBodyComponent::get_collision_layer)
            .method("set_collision_layer", &CharacterBodyComponent::set_collision_layer, {"layer"})
            .method("get_collision_mask", &CharacterBodyComponent::get_collision_mask)
            .method("set_collision_mask", &CharacterBodyComponent::set_collision_mask, {"mask"})
            .method("get_solver_ignore_mask", &CharacterBodyComponent::get_solver_ignore_mask)
            .method("set_solver_ignore_mask", &CharacterBodyComponent::set_solver_ignore_mask, {"mask"})
            .method("set_capsule", &CharacterBodyComponent::set_capsule, {"capsule"})
            .method("move_and_slide", &CharacterBodyComponent::move_and_slide, {"velocity", "fixed_dt"})
            .method("get_velocity", &CharacterBodyComponent::get_velocity)
            .method("set_velocity", &CharacterBodyComponent::set_velocity, {"velocity"})
            .method("get_position", &CharacterBodyComponent::get_position)
            .method("set_position", &CharacterBodyComponent::set_position, {"position"})
            .method("get_rotation", &CharacterBodyComponent::get_rotation)
            .method("set_rotation", &CharacterBodyComponent::set_rotation, {"radians"});

        bind_enum<InputEventType>(m_lua, m_meta, "InputEventType", {
                                      {"Axis", InputEventType::Axis},
                                      {"Pressed", InputEventType::Pressed},
                                      {"Released", InputEventType::Released},
                                  });

        bind_usertype<InputComponent>(m_lua, m_meta, "InputComponent", Bases<Component>{})
            .method_sig("bind_axis",
                        [](InputComponent& self, const std::string& name, const sol::protected_function& fn) {
                            return self.bind_axis(name.c_str(), [fn, name](float v) {
                                auto result = fn(v);
                                if (!result.valid()) {
                                    sol::error err = result;
                                    debug::log_error("Lua error in axis '{}' handler: {}", name, err.what());
                                }
                            });
                        }, "(name: string, fn: fun(value: number)): integer")
            .method_sig("unbind_axis",
                        [](InputComponent& self, const std::string& name, BindingId id) {
                            self.unbind_axis(name.c_str(), id);
                        }, "(name: string, id: integer)")
            .method_sig("bind_action",
                        [](InputComponent& self, const std::string& name, InputEventType type,
                           const sol::protected_function& fn) {
                            return self.bind_action(name.c_str(), type, [fn, name]() {
                                auto result = fn();
                                if (!result.valid()) {
                                    sol::error err = result;
                                    debug::log_error("Lua error in action '{}' handler: {}", name, err.what());
                                }
                            });
                        }, "(name: string, type: InputEventType, fn: fun()): integer")
            .method_sig("unbind_action",
                        [](InputComponent& self, const std::string& name, BindingId id) {
                            self.unbind_action(name.c_str(), id);
                        }, "(name: string, id: integer)")
            .method("clear_all_bindings", &InputComponent::clear_all_bindings);

        // Authorable-from-prefab registration. Each call registers (1) the
        // entity:add_<key>() method on the already-bound Entity usertype,
        // (2) the autocomplete entry, and (3) the prefab schema in one shot.
        // Order is load-bearing: Box2D bodies must be attached before colliders.
        bind_component_schema<RigidbodyComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "rigidbody", "add_rigidbody", {
                {"body_type", "set_body_type"},
                {"fixed_rotation", "set_fixed_rotation"},
            });

        bind_component_schema<CharacterBodyComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "character_body", "add_character_body", {
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"solver_ignore_mask", "set_solver_ignore_mask"},
                {"capsule", "set_capsule"},
            });

        bind_component_schema<BoxColliderComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "box_collider", "add_box_collider", {
                {"aabb", "set_aabb"},
                {"density", "set_density"},
                {"friction", "set_friction"},
                {"bounciness", "set_bounciness"},
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"trigger", "set_trigger"},
            });

        bind_component_schema<CapsuleColliderComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "capsule_collider", "add_capsule_collider", {
                {"capsule", "set_capsule"},
                {"density", "set_density"},
                {"friction", "set_friction"},
                {"bounciness", "set_bounciness"},
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"trigger", "set_trigger"},
            });

        bind_component_schema<CircleColliderComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "circle_collider", "add_circle_collider", {
                {"circle", "set_circle"},
                {"density", "set_density"},
                {"friction", "set_friction"},
                {"bounciness", "set_bounciness"},
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"trigger", "set_trigger"},
            });

        bind_component_schema<SpriteComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "sprite", "add_sprite", {
                {"texture", "set_texture"},
                {"pivot", "set_pivot"},
                {"scale", "set_scale"},
                {"tint", "set_tint"},
                {"z_index", "set_z_index"},
                {"pixels_per_meter", "set_pixels_per_meter"},
            });

        bind_component_schema<InputComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "input", "add_input", {});

        bind_component_schema<CameraComponent>(
            m_lua, m_meta, m_schemas, m_spawner, "camera", "add_camera", {});
    }
}
