#include "lua_script_system.h"

#include <algorithm>
#include <vector>

#include "app.h"
#include "assets.h"
#include "input.h"
#include "logging.h"
#include "path_utils.h"
#include "renderer.h"
#include "timer.h"
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
#include "engine/entity/entity.h"
#include "engine/entity/entity_spawner.h"
#include "engine/math/aabb.h"
#include "engine/math/capsule.h"
#include "engine/math/constants.h"
#include "engine/math/vector2.h"

namespace hob {
    LuaScriptSystem::LuaScriptSystem(App& app)
        : m_app(app)
        , m_lua() {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::string,
            sol::lib::math,
            sol::lib::table,
            sol::lib::io,
            sol::lib::os);

        register_bindings();
        run_bootstrap();
    }

    sol::state& LuaScriptSystem::get_lua() {
        return m_lua;
    }

    bool LuaScriptSystem::run_file(const std::filesystem::path& path) {
        const std::filesystem::path full_path = path.is_absolute() ? path : PathUtils::get_root_path() / path;

        auto result = m_lua.safe_script_file(full_path.string(), sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            debug::log_error("Lua error in {}: {}", full_path.string(), err.what());
            return false;
        }

        return true;
    }

    bool LuaScriptSystem::run_folder(const std::filesystem::path& path,
                                     std::initializer_list<std::string_view> excludes) {
        const std::filesystem::path root = path.is_absolute() ? path : PathUtils::get_root_path() / path;
        if (!std::filesystem::exists(root)) {
            debug::log_error("LuaScriptSystem::run_folder: '{}' does not exist", root.string());
            return false;
        }

        auto is_excluded = [&](const std::filesystem::path& p) {
            const std::filesystem::path rel = std::filesystem::relative(p, root);
            for (const auto& part : rel) {
                const std::string s = part.string();
                for (const auto& name : excludes) {
                    if (s == name) {
                        return true;
                    }
                }
            }

            return false;
        };

        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".lua") {
                continue;
            }

            if (is_excluded(entry.path())) {
                continue;
            }

            files.push_back(entry.path());
        }

        std::sort(files.begin(), files.end());

        bool all_ok = true;
        for (const auto& file : files) {
            auto result = m_lua.safe_script_file(file.string(), sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                debug::log_error("Lua error in {}: {}", file.string(), err.what());
                all_ok = false;
            }
        }

        return all_ok;
    }

    bool LuaScriptSystem::run_bootstrap() {
        // Engine bootstrap.
        // 1. Load engine scripts first (DefineComponent/DefinePrefab, schemas, shared globals).
        // 2. then everything user-defined under scripts - skipping the engine folder (already loaded) and main.lua.
        // 3. Then finally main.lua as the entry point.
        bool engine_ok = run_folder("scripts/engine");
        bool user_ok = run_folder("scripts", {"engine", "main.lua"});
        bool main_ok = run_file("scripts/main.lua");

        return engine_ok && user_ok && main_ok;
    }

    void LuaScriptSystem::register_bindings() {
        bind_math();
        bind_entity();
        bind_components();
        bind_subsystems();
        bind_logging();
    }

    void LuaScriptSystem::bind_math() {
        sol::table math_table = m_lua.create_named_table("Math");
        math_table["PI"] = PI;
        math_table["EPSILON"] = EPSILON;
        math_table["DEG_TO_RAD"] = DEG_TO_RAD;
        math_table["RAD_TO_DEG"] = RAD_TO_DEG;

        sol::table numbers_table = m_lua.create_named_table("Numbers");
        numbers_table["MIN_INT32"] = MIN_INT32;
        numbers_table["MAX_INT32"] = MAX_INT32;
        numbers_table["MAX_UINT32"] = MAX_UINT32;
        numbers_table["MIN_FLOAT"] = MIN_FLOAT;
        numbers_table["MAX_FLOAT"] = MAX_FLOAT;

        m_lua.new_usertype<Vector2>(
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

        sol::table vector2_table = m_lua["Vector2"];
        vector2_table["zero"] = &Vector2::zero;
        vector2_table["one"] = &Vector2::one;
        vector2_table["left"] = &Vector2::left;
        vector2_table["right"] = &Vector2::right;
        vector2_table["up"] = &Vector2::up;
        vector2_table["down"] = &Vector2::down;
        vector2_table["dot"] = &Vector2::dot;
        vector2_table["distance"] = &Vector2::distance;
        vector2_table["lerp"] = &Vector2::lerp;
        vector2_table["rotate_around"] = &Vector2::rotate_around;

        m_lua.new_usertype<Capsule>(
            "Capsule",
            sol::call_constructor, sol::constructors<Capsule(const Vector2&, const Vector2&, float)>(),
            "center_a", &Capsule::center_a,
            "center_b", &Capsule::center_b,
            "radius", &Capsule::radius,
            "get_height", &Capsule::get_height);

        m_lua.new_usertype<AABB>(
            "AABB",
            sol::call_constructor, sol::constructors<AABB(const Vector2&, const Vector2&)>(),
            "center", &AABB::center,
            "extents", &AABB::extents,
            "min", &AABB::min,
            "max", &AABB::max,
            "size", &AABB::size);

        m_lua.new_usertype<Color>(
            "Color",
            sol::call_constructor, sol::constructors<Color(), Color(float, float, float, float)>(),
            "r", &Color::r,
            "g", &Color::g,
            "b", &Color::b,
            "a", &Color::a,
            sol::meta_function::to_string, &Color::to_string);

        sol::table color_table = m_lua["Color"];
        color_table["black"] = &Color::black;
        color_table["white"] = &Color::white;
        color_table["gray"] = &Color::gray;
        color_table["red"] = &Color::red;
        color_table["green"] = &Color::green;
        color_table["blue"] = &Color::blue;
        color_table["yellow"] = &Color::yellow;
        color_table["magenta"] = &Color::magenta;
        color_table["cyan"] = &Color::cyan;
        color_table["orange"] = &Color::orange;
    }

    void LuaScriptSystem::bind_entity() {
        m_lua.new_usertype<Entity>(
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
            "add_lua_component", [](Entity& self, const std::string& class_name) {
                LuaScriptComponent* lua_script_comp = self.add_component<LuaScriptComponent>();
                lua_script_comp->set_class_name(class_name);
                return lua_script_comp;
            },
            // get_*
            "get_sprite", &Entity::get_component<SpriteComponent>,
            "get_camera", &Entity::get_component<CameraComponent>,
            "get_box_collider", &Entity::get_component<BoxColliderComponent>,
            "get_capsule_collider", &Entity::get_component<CapsuleColliderComponent>,
            "get_character_body", &Entity::get_component<CharacterBodyComponent>,
            "get_input", &Entity::get_component<InputComponent>,
            "get_lua_component", &Entity::get_component<LuaScriptComponent>);
    }

    void LuaScriptSystem::bind_components() {
        m_lua.new_usertype<Component>(
            "Component",
            sol::no_constructor,
            "get_entity", [](Component& c) { return &c.get_entity(); });

        m_lua.new_usertype<TransformComponent>(
            "TransformComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_position", &TransformComponent::get_position,
            "set_position", &TransformComponent::set_position,
            "get_rotation", &TransformComponent::get_rotation,
            "set_rotation", &TransformComponent::set_rotation,
            "get_scale", &TransformComponent::get_scale,
            "set_scale", &TransformComponent::set_scale);

        m_lua.new_usertype<SpriteComponent>(
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

        m_lua.new_usertype<CameraComponent>(
            "CameraComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "world_to_screen", sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::world_to_screen),
            "screen_to_world", sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::screen_to_world));

        m_lua.new_enum<BodyType>(
            "BodyType",
            {
                {"Static", BodyType::Static},
                {"Dynamic", BodyType::Dynamic},
                {"Kinematic", BodyType::Kinematic},
            });

        m_lua.new_usertype<RigidbodyComponent>(
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

        m_lua.new_usertype<ColliderComponent>(
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

        m_lua.new_usertype<BoxColliderComponent>(
            "BoxColliderComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<ColliderComponent, Component>(),
            "get_aabb", &BoxColliderComponent::get_aabb,
            "set_aabb", &BoxColliderComponent::set_aabb);

        m_lua.new_usertype<CapsuleColliderComponent>(
            "CapsuleColliderComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<ColliderComponent, Component>(),
            "get_capsule", &CapsuleColliderComponent::get_capsule,
            "set_capsule", &CapsuleColliderComponent::set_capsule);

        m_lua.new_usertype<CharacterBodyComponent>(
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

        m_lua.new_enum<InputEventType>(
            "InputEventType",
            {
                {"Axis", InputEventType::Axis},
                {"Pressed", InputEventType::Pressed},
                {"Released", InputEventType::Released},
            });

        m_lua.new_usertype<InputComponent>(
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

        m_lua.new_usertype<LuaScriptComponent>(
            "LuaScriptComponent",
            sol::no_constructor,
            sol::base_classes, sol::bases<Component>(),
            "get_class_name", &LuaScriptComponent::get_class_name,
            "set_class_name", &LuaScriptComponent::set_class_name);
    }

    void LuaScriptSystem::bind_subsystems() {
        // Capturing subsystems by reference is safe: App's destructor clears all
        // entities before any member destructor runs, so these lambdas can never be
        // invoked through Lua against a half-destroyed subsystem.
        EntitySpawner& spawner = m_app.get_entity_spawner();
        Input& input = m_app.get_input();
        Timer& timer = m_app.get_timer();
        Assets& assets = m_app.get_assets();

        sol::table entity_spawner_table = m_lua.create_named_table("EntitySpawner");
        entity_spawner_table["spawn_entity_c"] = [&spawner]() { return &spawner.spawn_entity(); };
        entity_spawner_table["destroy_entity"] = [&spawner](EntityId id) { spawner.destroy_entity(id); };
        entity_spawner_table["get_entity"] = [&spawner](EntityId id) { return spawner.get_entity(id); };

        sol::table input_table = m_lua.create_named_table("Input");
        input_table["get_mouse_screen_position"] = [&input]() { return input.get_mouse_screen_position(); };

        sol::table timer_table = m_lua.create_named_table("Timer");
        timer_table["get_fps"] = [&timer]() { return timer.get_fps(); };
        timer_table["set_fps"] = [&timer](uint32_t v) { timer.set_fps(v); };
        timer_table["get_time_scale"] = [&timer]() { return timer.get_time_scale(); };
        timer_table["set_time_scale"] = [&timer](float v) { timer.set_time_scale(v); };
        timer_table["get_play_time"] = [&timer]() { return timer.get_play_time(); };
        timer_table["get_delta_time"] = [&timer]() { return timer.get_delta_time(); };

        sol::table assets_table = m_lua.create_named_table("Assets");
        assets_table["load_texture"] = [&assets](const std::string& relative_path) {
            std::filesystem::path full = PathUtils::get_assets_root_path() / relative_path;
            return assets.load_texture(full);
        };

        sol::table camera_table = m_lua.create_named_table("Camera");
        camera_table["get_entity"] = [&spawner]() { return spawner.get_camera_entity(); };
        camera_table["world_to_screen"] = [&spawner](const Vector2& world_pos) {
            return spawner.get_camera_entity()->get_component<CameraComponent>()->world_to_screen(world_pos);
        };
        camera_table["screen_to_world"] = [&spawner](const Vector2& screen_pos) {
            return spawner.get_camera_entity()->get_component<CameraComponent>()->screen_to_world(screen_pos);
        };
        camera_table["get_position"] = [&spawner]() {
            return spawner.get_camera_entity()->get_transform()->get_position();
        };
        camera_table["set_position"] = [&spawner](const Vector2& p) {
            spawner.get_camera_entity()->get_transform()->set_position(p);
        };
    }

    void LuaScriptSystem::bind_logging() {
        auto stringify_args = [](sol::this_state ts, sol::variadic_args args) -> std::string {
            lua_State* L = ts;
            sol::state_view sv(L);
            sol::protected_function tostring = sv["tostring"];
            std::string out;
            bool first = true;
            for (auto v : args) {
                sol::protected_function_result r = tostring(sol::object(v));
                std::string piece = r.valid() ? r.get<std::string>() : "<tostring failed>";
                if (!first) {
                    out += '\t';
                }
                out += piece;
                first = false;
            }
            return out;
        };
        m_lua.set_function("log", [stringify_args](sol::this_state ts, sol::variadic_args args) {
            debug::log("{}", stringify_args(ts, args));
        });
        m_lua.set_function("log_error", [stringify_args](sol::this_state ts, sol::variadic_args args) {
            debug::log_error("{}", stringify_args(ts, args));
        });
    }
}
