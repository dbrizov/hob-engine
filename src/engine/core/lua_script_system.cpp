#include "lua_script_system.h"

#include <algorithm>
#include <vector>

#include "app.h"
#include "assets.h"
#include "debug.h"
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
    // clang-format off
    HOB_LUA_TYPE(Vector2, "Vector2")
    HOB_LUA_TYPE(Capsule, "Capsule")
    HOB_LUA_TYPE(AABB, "AABB")
    HOB_LUA_TYPE(Color, "Color")
    HOB_LUA_TYPE(EntityHandle, "Entity")
    HOB_LUA_TYPE(Component, "Component")
    HOB_LUA_TYPE(TransformComponent, "TransformComponent")
    HOB_LUA_TYPE(SpriteComponent, "SpriteComponent")
    HOB_LUA_TYPE(CameraComponent, "CameraComponent")
    HOB_LUA_TYPE(RigidbodyComponent, "RigidbodyComponent")
    HOB_LUA_TYPE(ColliderComponent, "ColliderComponent")
    HOB_LUA_TYPE(BoxColliderComponent, "BoxColliderComponent")
    HOB_LUA_TYPE(CapsuleColliderComponent, "CapsuleColliderComponent")
    HOB_LUA_TYPE(CharacterBodyComponent, "CharacterBodyComponent")
    HOB_LUA_TYPE(InputComponent, "InputComponent")
    HOB_LUA_TYPE(BodyType, "BodyType")
    HOB_LUA_TYPE(InputEventType, "InputEventType")
    // clang-format on

    LuaScriptSystem::LuaScriptSystem(App& app)
        : m_app(app)
        , m_lua() {
        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::string,
            sol::lib::math,
            sol::lib::table,
            sol::lib::io,
            sol::lib::os,
            sol::lib::package,
            sol::lib::coroutine,
            sol::lib::debug);

        // Make `require` find modules in scripts/lib (e.g. vendored lldebugger).
        const std::string lib_path = (PathUtils::get_root_path() / "scripts" / "lib" / "?.lua").string();
        sol::table package = m_lua["package"];
        package["path"] = lib_path + ";" + package["path"].get<std::string>();

        register_bindings();
#ifndef NDEBUG
        dump_meta();
#endif
        run_bootstrap();
    }

    sol::state& LuaScriptSystem::get_lua() {
        return m_lua;
    }

    bool LuaScriptSystem::run_file(const std::filesystem::path& relative_path) {
        const std::filesystem::path full_path = PathUtils::get_root_path() / relative_path;

        auto result = m_lua.safe_script_file(full_path.string(), sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            debug::log_error("Lua error in {}: {}", full_path.string(), err.what());
            return false;
        }

        return true;
    }

    bool LuaScriptSystem::run_folder(const std::filesystem::path& relative_path,
                                     const std::vector<std::string>& excludes) {
        const std::filesystem::path root = PathUtils::get_root_path() / relative_path;
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
        return run_file("scripts/engine/bootstrap.lua");
    }

    void LuaScriptSystem::register_bindings() {
        bind_math();
        bind_entity();
        bind_components();
        bind_subsystems();
        bind_debug();
    }

    void LuaScriptSystem::dump_meta() {
        const std::filesystem::path out_path = PathUtils::get_root_path() / "scripts" / "meta" / "engine.generated.lua";
        if (m_meta.write_to_file(out_path)) {
            debug::log("Wrote Lua meta annotations to {}", out_path.string());
        }
    }

    void LuaScriptSystem::bind_math() {
        bind_table(m_lua, m_meta, "Math")
            .constant("PI", PI)
            .constant("EPSILON", EPSILON)
            .constant("DEG_TO_RAD", DEG_TO_RAD)
            .constant("RAD_TO_DEG", RAD_TO_DEG)
            .constant("MIN_INTEGER", MIN_INT64)
            .constant("MAX_INTEGER", MAX_INT64)
            .constant("MIN_NUMBER", MIN_DOUBLE)
            .constant("MAX_NUMBER", MAX_DOUBLE);

        bind_usertype<Vector2>(m_lua, m_meta, "Vector2")
            .ctors<sol::types<>, sol::types<float, float>>()
            .field("x", &Vector2::x)
            .field("y", &Vector2::y)
            .method("length", &Vector2::length)
            .method("length_sqr", &Vector2::length_sqr)
            .method("normalized", &Vector2::normalized)
            .op_add(&Vector2::operator+)
            .op_sub(sol::resolve<Vector2(const Vector2&) const>(&Vector2::operator-))
            .op_unm(sol::resolve<Vector2() const>(&Vector2::operator-))
            .op_mul(&Vector2::operator*)
            .op_div(&Vector2::operator/)
            .op_eq(&Vector2::operator==)
            .op_tostring(&Vector2::to_string)
            .method("zero", &Vector2::zero)
            .method("one", &Vector2::one)
            .method("left", &Vector2::left)
            .method("right", &Vector2::right)
            .method("up", &Vector2::up)
            .method("down", &Vector2::down)
            .method("dot", &Vector2::dot, {"a", "b"})
            .method("distance", &Vector2::distance, {"a", "b"})
            .method("lerp", &Vector2::lerp, {"a", "b", "t"})
            .method("rotate_around", &Vector2::rotate_around, {"point", "pivot", "radians"});

        bind_usertype<Capsule>(m_lua, m_meta, "Capsule")
            .ctors<sol::types<const Vector2&, const Vector2&, float>>()
            .field("center_a", &Capsule::center_a)
            .field("center_b", &Capsule::center_b)
            .field("radius", &Capsule::radius)
            .method("get_height", &Capsule::get_height);

        bind_usertype<AABB>(m_lua, m_meta, "AABB")
            .ctors<sol::types<const Vector2&, const Vector2&>>()
            .field("center", &AABB::center)
            .field("extents", &AABB::extents)
            .method("min", &AABB::min)
            .method("max", &AABB::max)
            .method("size", &AABB::size);

        bind_usertype<Color>(m_lua, m_meta, "Color")
            .ctors<sol::types<>, sol::types<float, float, float, float>>()
            .field("r", &Color::r)
            .field("g", &Color::g)
            .field("b", &Color::b)
            .field("a", &Color::a)
            .op_tostring(&Color::to_string)
            .method("black", &Color::black)
            .method("white", &Color::white)
            .method("gray", &Color::gray)
            .method("red", &Color::red)
            .method("green", &Color::green)
            .method("blue", &Color::blue)
            .method("yellow", &Color::yellow)
            .method("magenta", &Color::magenta)
            .method("cyan", &Color::cyan)
            .method("orange", &Color::orange);
    }

    void LuaScriptSystem::bind_entity() {
        const EntitySpawner& spawner = m_app.get_entity_spawner();

        auto get_entity = [&spawner](const EntityHandle& h) -> Entity* {
            return spawner.get_entity(h.id);
        };

        bind_usertype<EntityHandle>(m_lua, m_meta, "Entity")
            .method("get_id", [](const EntityHandle& h) { return h.id; })
            .method("is_valid", [get_entity](const EntityHandle& h) { return get_entity(h) != nullptr; })
            .method("is_in_play", [get_entity](const EntityHandle& h) {
                Entity* e = get_entity(h);
                return e != nullptr && e->is_in_play();
            })
            .method("is_ticking", [get_entity](const EntityHandle& h) {
                Entity* e = get_entity(h);
                return e != nullptr && e->is_ticking();
            })
            .method("set_ticking", [get_entity](const EntityHandle& h, bool v) {
                if (Entity* e = get_entity(h)) {
                    e->set_ticking(v);
                }
            }, {"ticking"})
            .method("add_rigidbody", [get_entity](const EntityHandle& h) -> RigidbodyComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<RigidbodyComponent>() : nullptr;
            })
            .method("add_box_collider", [get_entity](const EntityHandle& h) -> BoxColliderComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<BoxColliderComponent>() : nullptr;
            })
            .method("add_capsule_collider", [get_entity](const EntityHandle& h) -> CapsuleColliderComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<CapsuleColliderComponent>() : nullptr;
            })
            .method("add_character_body", [get_entity](const EntityHandle& h) -> CharacterBodyComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<CharacterBodyComponent>() : nullptr;
            })
            .method("add_sprite", [get_entity](const EntityHandle& h) -> SpriteComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<SpriteComponent>() : nullptr;
            })
            .method("add_input", [get_entity](const EntityHandle& h) -> InputComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<InputComponent>() : nullptr;
            })
            .method_sig(
                "add_lua_component",
                [get_entity](const EntityHandle& h, const std::string& class_name) -> sol::object {
                    Entity* e = get_entity(h);
                    if (e == nullptr) {
                        return sol::lua_nil;
                    }

                    LuaScriptComponent* lua_comp = e->add_component<LuaScriptComponent>(class_name);
                    if (lua_comp == nullptr) {
                        return sol::lua_nil;
                    }

                    return lua_comp->get_lua_instance();
                }, "(class_name: string): LuaComponent?")
            .method("get_transform", [get_entity](const EntityHandle& h) -> TransformComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_transform() : nullptr;
            })
            .method("get_rigidbody", [get_entity](const EntityHandle& h) -> RigidbodyComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_rigidbody() : nullptr;
            })
            .method("get_box_collider", [get_entity](const EntityHandle& h) -> BoxColliderComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_component<BoxColliderComponent>() : nullptr;
            })
            .method("get_capsule_collider", [get_entity](const EntityHandle& h) -> CapsuleColliderComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_component<CapsuleColliderComponent>() : nullptr;
            })
            .method("get_character_body", [get_entity](const EntityHandle& h) -> CharacterBodyComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_component<CharacterBodyComponent>() : nullptr;
            })
            .method("get_sprite", [get_entity](const EntityHandle& h) -> SpriteComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_component<SpriteComponent>() : nullptr;
            })
            .method("get_input", [get_entity](const EntityHandle& h) -> InputComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_component<InputComponent>() : nullptr;
            })
            .method_sig("get_lua_component",
                        [get_entity](const EntityHandle& h, const std::string& class_name) -> sol::object {
                            Entity* e = get_entity(h);
                            if (e == nullptr) {
                                return sol::lua_nil;
                            }

                            for (LuaScriptComponent* lua_comp : e->get_components<LuaScriptComponent>()) {
                                if (lua_comp->get_class_name() == class_name) {
                                    return lua_comp->get_lua_instance();
                                }
                            }

                            return sol::lua_nil;
                        }, "(class_name: string): LuaComponent?")
            .method_sig("get_lua_components",
                        [this, get_entity](const EntityHandle& h) {
                            sol::table out = m_lua.create_table();
                            Entity* e = get_entity(h);
                            if (e == nullptr) {
                                return out;
                            }

                            for (LuaScriptComponent* lua_comp : e->get_components<LuaScriptComponent>()) {
                                out.add(lua_comp->get_lua_instance());
                            }

                            return out;
                        }, "(): LuaComponent[]")
            .op_tostring([get_entity](const EntityHandle& h) {
                Entity* e = get_entity(h);
                return e ? e->to_string() : std::format("Entity(invalid, id = {})", h.id);
            });
    }

    void LuaScriptSystem::bind_components() {
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
            .method("get_texture_id", &SpriteComponent::get_texture_id)
            .method("set_texture_id", &SpriteComponent::set_texture_id, {"id"})
            .method("get_pivot", &SpriteComponent::get_pivot)
            .method("set_pivot", &SpriteComponent::set_pivot, {"pivot"})
            .method("get_scale", &SpriteComponent::get_scale)
            .method("set_scale", &SpriteComponent::set_scale, {"scale"})
            .method("get_tint", &SpriteComponent::get_tint)
            .method("set_tint", &SpriteComponent::set_tint, {"color"})
            .method("get_z_index", &SpriteComponent::get_z_index)
            .method("set_z_index", &SpriteComponent::set_z_index, {"z_index"});

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
            .method("set_trigger", &ColliderComponent::set_trigger, {"trigger"});

        bind_usertype<BoxColliderComponent>(m_lua, m_meta, "BoxColliderComponent",
                                            Bases<ColliderComponent, Component>{})
            .method("get_aabb", &BoxColliderComponent::get_aabb)
            .method("set_aabb", &BoxColliderComponent::set_aabb, {"aabb"});

        bind_usertype<CapsuleColliderComponent>(m_lua, m_meta, "CapsuleColliderComponent",
                                                Bases<ColliderComponent, Component>{})
            .method("get_capsule", &CapsuleColliderComponent::get_capsule)
            .method("set_capsule", &CapsuleColliderComponent::set_capsule, {"capsule"});

        bind_usertype<CharacterBodyComponent>(m_lua, m_meta, "CharacterBodyComponent", Bases<Component>{})
            .method("get_collision_layer", &CharacterBodyComponent::get_collision_layer)
            .method("set_collision_layer", &CharacterBodyComponent::set_collision_layer, {"layer"})
            .method("get_collision_mask", &CharacterBodyComponent::get_collision_mask)
            .method("set_collision_mask", &CharacterBodyComponent::set_collision_mask, {"mask"})
            .method("get_solver_ignore_mask", &CharacterBodyComponent::get_solver_ignore_mask)
            .method("set_solver_ignore_mask", &CharacterBodyComponent::set_solver_ignore_mask, {"mask"})
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
    }

    void LuaScriptSystem::bind_subsystems() {
        // Capturing subsystems by reference is safe: App's destructor clears all
        // entities before any member destructor runs, so these lambdas can never be
        // invoked through Lua against a half-destroyed subsystem.
        EntitySpawner& spawner = m_app.get_entity_spawner();
        Input& input = m_app.get_input();
        Timer& timer = m_app.get_timer();
        Assets& assets = m_app.get_assets();

        bind_table(m_lua, m_meta, "Scripts")
            .fn("run_file", [this](const std::string& relative_path) {
                return run_file(relative_path);
            }, {"relative_path"})
            .fn("run_folder", [this](const std::string& relative_path, sol::optional<sol::table> excludes) {
                std::vector<std::string> exclude_list;
                if (excludes) {
                    for (const auto& kv : *excludes) {
                        exclude_list.push_back(kv.second.as<std::string>());
                    }
                }

                return run_folder(relative_path, exclude_list);
            }, {"relative_path", "excludes"});

        bind_table(m_lua, m_meta, "EntitySpawner")
            .fn("spawn_entity_c", [&spawner]() { return EntityHandle(spawner.spawn_entity().get_id()); })
            .fn("destroy_entity", [&spawner](const EntityHandle& h) { spawner.destroy_entity(h.id); }, {"entity"})
            .fn("get_entity", [](EntityId id) { return EntityHandle(id); }, {"id"});

        bind_table(m_lua, m_meta, "Input")
            .fn("get_mouse_screen_position", [&input]() { return input.get_mouse_screen_position(); });

        bind_table(m_lua, m_meta, "Timer")
            .fn("get_fps", [&timer]() { return timer.get_fps(); })
            .fn("set_fps", [&timer](uint32_t v) { timer.set_fps(v); }, {"fps"})
            .fn("get_time_scale", [&timer]() { return timer.get_time_scale(); })
            .fn("set_time_scale", [&timer](float v) { timer.set_time_scale(v); }, {"scale"})
            .fn("get_play_time", [&timer]() { return timer.get_play_time(); })
            .fn("get_delta_time", [&timer]() { return timer.get_delta_time(); });

        bind_table(m_lua, m_meta, "Assets")
            .fn("load_texture", [&assets](const std::string& relative_path) {
                std::filesystem::path full = PathUtils::get_assets_root_path() / relative_path;
                return assets.load_texture(full);
            }, {"relative_path"});

        bind_table(m_lua, m_meta, "Camera")
            .fn("get_entity", [&spawner]() {
                return EntityHandle(spawner.get_camera_entity()->get_id());
            })
            .fn("world_to_screen", [&spawner](const Vector2& world_pos) {
                return spawner.get_camera_entity()->get_component<CameraComponent>()->world_to_screen(world_pos);
            }, {"world_pos"})
            .fn("screen_to_world", [&spawner](const Vector2& screen_pos) {
                return spawner.get_camera_entity()->get_component<CameraComponent>()->screen_to_world(screen_pos);
            }, {"screen_pos"})
            .fn("get_position", [&spawner]() {
                return spawner.get_camera_entity()->get_transform()->get_position();
            })
            .fn("set_position", [&spawner](const Vector2& p) {
                spawner.get_camera_entity()->get_transform()->set_position(p);
            }, {"position"});
    }

    void LuaScriptSystem::bind_debug() {
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

        bind_table(m_lua, m_meta, "Debug")
            .fn_sig("log",
                    [stringify_args](sol::this_state ts, sol::variadic_args args) {
                        debug::log("{}", stringify_args(ts, args));
                    }, "(...: any)")
            .fn_sig("log_error",
                    [stringify_args](sol::this_state ts, sol::variadic_args args) {
                        debug::log_error("{}", stringify_args(ts, args));
                    }, "(...: any)")
            .fn_sig("draw_line",
                    [](const Vector2& from, const Vector2& to, const Color& color,
                       sol::optional<float> thickness) {
                        debug::draw_line(from, to, color, thickness.value_or(2.0f));
                    }, "(from: Vector2, to: Vector2, color: Color, thickness: number?)")
            .fn_sig("draw_circle",
                    [](const Vector2& center, float radius, const Color& color,
                       sol::optional<float> thickness, sol::optional<int> segments) {
                        debug::draw_circle(center, radius, color, thickness.value_or(2.0f), segments.value_or(16));
                    }, "(center: Vector2, radius: number, color: Color, thickness: number?, segments: integer?)");
    }
}
