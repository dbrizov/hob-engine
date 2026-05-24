#include "lua_bindings.h"

#include <sol/sol.hpp>

#include "engine/components/camera_component.h"
#include "engine/components/input_component.h"
#include "engine/components/lua_script_component.h"
#include "engine/components/physics/box_collider_component.h"
#include "engine/components/physics/capsule_collider_component.h"
#include "engine/components/physics/character_body_component.h"
#include "engine/components/physics/collider_component.h"
#include "engine/components/physics/rigidbody_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/app.h"
#include "engine/core/assets.h"
#include "engine/core/input.h"
#include "engine/core/logging.h"
#include "engine/core/path_utils.h"
#include "engine/core/renderer.h"
#include "engine/core/timer.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_spawner.h"
#include "engine/math/aabb.h"
#include "engine/math/capsule.h"
#include "engine/math/constants.h"
#include "engine/math/vector2.h"

namespace hob {
    static void bind_math(sol::state& lua) {
        lua.new_usertype<Vector2>(
            "Vector2",
            sol::call_constructor, sol::constructors<Vector2(), Vector2(float, float)>(),
            "x", &Vector2::x,
            "y", &Vector2::y,
            "length", &Vector2::length,
            "length_sqr", &Vector2::length_sqr,
            "normalized", &Vector2::normalized,
            "to_string", &Vector2::to_string,
            sol::meta_function::addition, &Vector2::operator+,
            sol::meta_function::subtraction, sol::resolve<Vector2(const Vector2&) const>(&Vector2::operator-),
            sol::meta_function::unary_minus, sol::resolve<Vector2() const>(&Vector2::operator-),
            sol::meta_function::multiplication, &Vector2::operator*,
            sol::meta_function::division, &Vector2::operator/,
            sol::meta_function::equal_to, &Vector2::operator==,
            sol::meta_function::to_string, &Vector2::to_string);

        sol::table v2 = lua["Vector2"];
        v2["zero"] = &Vector2::zero;
        v2["one"] = &Vector2::one;
        v2["left"] = &Vector2::left;
        v2["right"] = &Vector2::right;
        v2["up"] = &Vector2::up;
        v2["down"] = &Vector2::down;
        v2["dot"] = &Vector2::dot;
        v2["distance"] = &Vector2::distance;
        v2["lerp"] = &Vector2::lerp;
        v2["rotate_around"] = &Vector2::rotate_around;

        lua.new_usertype<Capsule>(
            "Capsule",
            sol::call_constructor, sol::constructors<Capsule(const Vector2&, const Vector2&, float)>(),
            "center_a", &Capsule::center_a,
            "center_b", &Capsule::center_b,
            "radius", &Capsule::radius,
            "get_height", &Capsule::get_height);

        lua.new_usertype<AABB>(
            "AABB",
            sol::call_constructor, sol::constructors<AABB(const Vector2&, const Vector2&)>(),
            "center", &AABB::center,
            "extents", &AABB::extents,
            "min", &AABB::min,
            "max", &AABB::max,
            "size", &AABB::size);

        lua.new_usertype<Color>(
            "Color",
            sol::call_constructor, sol::constructors<Color(), Color(float, float, float, float)>(),
            "r", &Color::r,
            "g", &Color::g,
            "b", &Color::b,
            "a", &Color::a);

        sol::table c = lua["Color"];
        c["black"] = &Color::black;
        c["white"] = &Color::white;
        c["gray"] = &Color::gray;
        c["red"] = &Color::red;
        c["green"] = &Color::green;
        c["blue"] = &Color::blue;
        c["yellow"] = &Color::yellow;
        c["magenta"] = &Color::magenta;
        c["cyan"] = &Color::cyan;
        c["orange"] = &Color::orange;

        lua["DEG_TO_RAD"] = DEG_TO_RAD;
        lua["RAD_TO_DEG"] = RAD_TO_DEG;
        lua["PI"] = PI;
    }

    static void bind_components(sol::state& lua) {
        lua.new_usertype<Component>(
            "Component",
            sol::no_constructor,
            "get_entity", [](Component& c) { return &c.get_entity(); });

        lua.new_usertype<TransformComponent>(
            "TransformComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_position", &TransformComponent::get_position,
            "set_position", &TransformComponent::set_position,
            "get_rotation", &TransformComponent::get_rotation,
            "set_rotation", &TransformComponent::set_rotation,
            "get_scale", &TransformComponent::get_scale,
            "set_scale", &TransformComponent::set_scale);

        lua.new_usertype<SpriteComponent>(
            "SpriteComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_texture_id", &SpriteComponent::get_texture_id,
            "set_texture_id", &SpriteComponent::set_texture_id,
            "get_pivot", &SpriteComponent::get_pivot,
            "set_pivot", &SpriteComponent::set_pivot,
            "get_scale", &SpriteComponent::get_scale,
            "set_scale", &SpriteComponent::set_scale,
            "get_tint", &SpriteComponent::get_tint,
            "set_tint", &SpriteComponent::set_tint,
            "get_z_index", &SpriteComponent::get_z_index,
            "set_z_index", &SpriteComponent::set_z_index);

        lua.new_usertype<CameraComponent>(
            "CameraComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "world_to_screen", sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::world_to_screen),
            "screen_to_world", sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::screen_to_world));

        lua.new_enum<BodyType>(
            "BodyType",
            {
                {"Static", BodyType::Static},
                {"Dynamic", BodyType::Dynamic},
                {"Kinematic", BodyType::Kinematic},
            });

        lua.new_usertype<RigidbodyComponent>(
            "RigidbodyComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_body_type", &RigidbodyComponent::get_body_type,
            "set_body_type", &RigidbodyComponent::set_body_type,
            "has_fixed_rotation", &RigidbodyComponent::has_fixed_rotation,
            "set_fixed_rotation", &RigidbodyComponent::set_fixed_rotation,
            "get_velocity", &RigidbodyComponent::get_velocity,
            "set_velocity", &RigidbodyComponent::set_velocity,
            "get_position", &RigidbodyComponent::get_position,
            "set_position", &RigidbodyComponent::set_position,
            "get_rotation", &RigidbodyComponent::get_rotation,
            "set_rotation", &RigidbodyComponent::set_rotation);

        lua.new_usertype<ColliderComponent>(
            "ColliderComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_density", &ColliderComponent::get_density,
            "set_density", &ColliderComponent::set_density,
            "get_friction", &ColliderComponent::get_friction,
            "set_friction", &ColliderComponent::set_friction,
            "get_bounciness", &ColliderComponent::get_bounciness,
            "set_bounciness", &ColliderComponent::set_bounciness,
            "get_collision_layer", &ColliderComponent::get_collision_layer,
            "set_collision_layer", &ColliderComponent::set_collision_layer,
            "get_collision_mask", &ColliderComponent::get_collision_mask,
            "set_collision_mask", &ColliderComponent::set_collision_mask,
            "is_trigger", &ColliderComponent::is_trigger,
            "set_trigger", &ColliderComponent::set_trigger);

        lua.new_usertype<BoxColliderComponent>(
            "BoxColliderComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<ColliderComponent, Component>(),
            "get_aabb", &BoxColliderComponent::get_aabb,
            "set_aabb", &BoxColliderComponent::set_aabb);

        lua.new_usertype<CapsuleColliderComponent>(
            "CapsuleColliderComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<ColliderComponent, Component>(),
            "get_capsule", &CapsuleColliderComponent::get_capsule,
            "set_capsule", &CapsuleColliderComponent::set_capsule);

        lua.new_usertype<CharacterBodyComponent>(
            "CharacterBodyComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_collision_layer", &CharacterBodyComponent::get_collision_layer,
            "set_collision_layer", &CharacterBodyComponent::set_collision_layer,
            "get_collision_mask", &CharacterBodyComponent::get_collision_mask,
            "set_collision_mask", &CharacterBodyComponent::set_collision_mask,
            "get_solver_ignore_mask", &CharacterBodyComponent::get_solver_ignore_mask,
            "set_solver_ignore_mask", &CharacterBodyComponent::set_solver_ignore_mask,
            "move_and_slide", &CharacterBodyComponent::move_and_slide,
            "get_velocity", &CharacterBodyComponent::get_velocity,
            "set_velocity", &CharacterBodyComponent::set_velocity,
            "get_position", &CharacterBodyComponent::get_position,
            "set_position", &CharacterBodyComponent::set_position,
            "get_rotation", &CharacterBodyComponent::get_rotation,
            "set_rotation", &CharacterBodyComponent::set_rotation);

        lua.new_enum<InputEventType>(
            "InputEventType",
            {
                {"Axis", InputEventType::Axis},
                {"Pressed", InputEventType::Pressed},
                {"Released", InputEventType::Released},
            });

        lua.new_usertype<InputComponent>(
            "InputComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "bind_axis", [](InputComponent& self, const std::string& name, sol::protected_function fn) {
                std::string captured_name = name;
                return self.bind_axis(captured_name.c_str(), [fn, captured_name](float v) {
                    auto result = fn(v);
                    if (!result.valid()) {
                        sol::error err = result;
                        debug::log_error("Lua error in axis '{}' handler: {}", captured_name, err.what());
                    }
                });
            },
            "unbind_axis", [](InputComponent& self, const std::string& name, BindingId id) {
                self.unbind_axis(name.c_str(), id);
            },
            "bind_action", [](InputComponent& self, const std::string& name, InputEventType type,
                              sol::protected_function fn) {
                std::string captured_name = name;
                return self.bind_action(captured_name.c_str(), type, [fn, captured_name]() {
                    auto result = fn();
                    if (!result.valid()) {
                        sol::error err = result;
                        debug::log_error("Lua error in action '{}' handler: {}", captured_name, err.what());
                    }
                });
            },
            "unbind_action", [](InputComponent& self, const std::string& name, BindingId id) {
                self.unbind_action(name.c_str(), id);
            },
            "clear_all_bindings", &InputComponent::clear_all_bindings);

        lua.new_usertype<LuaScriptComponent>(
            "LuaScriptComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_script_name", &LuaScriptComponent::get_script_name,
            "set_script_name", &LuaScriptComponent::set_script_name);
    }

    static void bind_entity(sol::state& lua) {
        lua.new_usertype<Entity>(
            "Entity",
            sol::no_constructor,
            "get_id", &Entity::get_id,
            "is_in_play", &Entity::is_in_play,
            "is_ticking", &Entity::is_ticking,
            "set_ticking", &Entity::set_ticking,
            "get_transform", &Entity::get_transform,
            "get_rigidbody", &Entity::get_rigidbody,
            // add_*
            "add_sprite", &Entity::add_component<SpriteComponent>,
            "add_camera", &Entity::add_component<CameraComponent>,
            "add_rigidbody", &Entity::add_component<RigidbodyComponent>,
            "add_box_collider", &Entity::add_component<BoxColliderComponent>,
            "add_capsule_collider", &Entity::add_component<CapsuleColliderComponent>,
            "add_character_body", &Entity::add_component<CharacterBodyComponent>,
            "add_input", &Entity::add_component<InputComponent>,
            "add_lua_script", [](Entity& self, const std::string& script_name) {
                LuaScriptComponent* lua_script_comp = self.add_component<LuaScriptComponent>();
                lua_script_comp->set_script_name(script_name);
                return lua_script_comp;
            },
            // get_*
            "get_sprite", &Entity::get_component<SpriteComponent>,
            "get_camera", &Entity::get_component<CameraComponent>,
            "get_box_collider", &Entity::get_component<BoxColliderComponent>,
            "get_capsule_collider", &Entity::get_component<CapsuleColliderComponent>,
            "get_character_body", &Entity::get_component<CharacterBodyComponent>,
            "get_input", &Entity::get_component<InputComponent>,
            "get_lua_script", &Entity::get_component<LuaScriptComponent>);

        lua.new_usertype<EntitySpawner>(
            "EntitySpawner",
            sol::no_constructor,
            "spawn_entity", [](EntitySpawner& self) { return &self.spawn_entity(); },
            "destroy_entity", &EntitySpawner::destroy_entity,
            "get_entity", &EntitySpawner::get_entity,
            "get_camera_entity", &EntitySpawner::get_camera_entity);
    }

    static void bind_subsystems(sol::state& lua) {
        lua.new_usertype<Assets>(
            "Assets",
            sol::no_constructor,
            "load_texture", [](Assets& self, const std::string& relative_path) {
                std::filesystem::path full = PathUtils::get_assets_root_path() / relative_path;
                return self.load_texture(full);
            });

        lua.new_usertype<Input>(
            "Input",
            sol::no_constructor,
            "get_mouse_screen_position", &Input::get_mouse_screen_position);

        lua.new_usertype<Timer>(
            "Timer",
            sol::no_constructor,
            "get_fps", &Timer::get_fps,
            "set_fps", &Timer::set_fps,
            "get_time_scale", &Timer::get_time_scale,
            "set_time_scale", &Timer::set_time_scale,
            "get_play_time", &Timer::get_play_time,
            "get_delta_time", &Timer::get_delta_time);

        lua.new_usertype<App>(
            "App",
            sol::no_constructor,
            "get_assets", [](App& self) { return &self.get_assets(); },
            "get_input", [](App& self) { return &self.get_input(); },
            "get_timer", [](App& self) { return &self.get_timer(); },
            "get_entity_spawner", [](App& self) { return &self.get_entity_spawner(); });
    }

    void register_bindings(sol::state& lua, App& app) {
        bind_math(lua);
        bind_components(lua);
        bind_entity(lua);
        bind_subsystems(lua);

        // Convenience globals.
        lua["app"] = &app;

        // Logging hooks.
        lua.set_function("log", [](const std::string& msg) {
            debug::log("{}", msg);
        });
        lua.set_function("log_error", [](const std::string& msg) {
            debug::log_error("{}", msg);
        });
    }
}
