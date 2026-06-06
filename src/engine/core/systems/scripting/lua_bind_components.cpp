#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_schema_component.h"
#include "lua_schema_factory.h"
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
#include "engine/animation/animation_clip.h"
#include "engine/components/sprite_animator_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/debug.h"
#include "engine/core/engine.h"
#include "engine/core/systems/input.h"
#include "engine/core/systems/renderer/renderer.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_ref.h"

namespace hob {
    void LuaScriptSystem::bind_components() {
        sol::state& lua = m_impl->lua;
        LuaMetaRegistry& meta = m_impl->meta;
        LuaComponentSchemaRegistry& schemas = m_impl->component_schemas;
        LuaFactorySchemaRegistry& factory_schemas = m_impl->factory_schemas;

        bind_usertype<Component>(lua, meta)
            .method("get_entity", [](Component& c) {
                return EntityRef(c.get_entity().get_id(), c.get_engine().get_entity_spawner());
            })
            .op_tostring(&Component::to_string)
            .op_concat(&Component::to_string);

        bind_usertype<TransformComponent>(lua, meta, Bases<Component>{})
            .method("get_position", &TransformComponent::get_position)
            .method("set_position", &TransformComponent::set_position, {"position"})
            .method("get_rotation", &TransformComponent::get_rotation)
            .method("set_rotation", &TransformComponent::set_rotation, {"radians"})
            .method("get_scale", &TransformComponent::get_scale)
            .method("set_scale", &TransformComponent::set_scale, {"scale"})
            .method("get_interpolate_physics", &TransformComponent::get_interpolate_physics)
            .method("set_interpolate_physics", &TransformComponent::set_interpolate_physics, {"value"});

        bind_enum<BodyType>(lua, meta, {
                                {"Static", BodyType::Static},
                                {"Dynamic", BodyType::Dynamic},
                                {"Kinematic", BodyType::Kinematic},
                            });

        bind_usertype<RigidbodyComponent>(lua, meta, Bases<Component>{})
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

        bind_usertype<CharacterBodyComponent>(lua, meta, Bases<Component>{})
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

        bind_usertype<ColliderComponent>(lua, meta, Bases<Component>{})
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

        bind_usertype<BoxColliderComponent>(lua, meta, Bases<ColliderComponent, Component>{})
            .method("get_aabb", &BoxColliderComponent::get_aabb)
            .method("set_aabb", &BoxColliderComponent::set_aabb, {"aabb"})
            .method("get_scaled_aabb", &BoxColliderComponent::get_scaled_aabb);

        bind_usertype<CapsuleColliderComponent>(lua, meta, Bases<ColliderComponent, Component>{})
            .method("get_capsule", &CapsuleColliderComponent::get_capsule)
            .method("set_capsule", &CapsuleColliderComponent::set_capsule, {"capsule"})
            .method("get_scaled_capsule", &CapsuleColliderComponent::get_scaled_capsule);

        bind_usertype<CircleColliderComponent>(lua, meta, Bases<ColliderComponent, Component>{})
            .method("get_circle", &CircleColliderComponent::get_circle)
            .method("set_circle", &CircleColliderComponent::set_circle, {"circle"})
            .method("get_scaled_circle", &CircleColliderComponent::get_scaled_circle);

        bind_enum<InputEventType>(lua, meta, {
                                      {"Axis", InputEventType::Axis},
                                      {"Pressed", InputEventType::Pressed},
                                      {"Released", InputEventType::Released},
                                  });

        bind_usertype<InputComponent>(lua, meta, Bases<Component>{})
            .method_sig("bind_axis",
                        [](InputComponent& self, const std::string& name, const sol::protected_function& fn) {
                            return self.bind_axis(name.c_str(), [fn, name](float v) {
                                auto result = fn(v);
                                if (!result.valid()) {
                                    const sol::error err = result;
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
                                    const sol::error err = result;
                                    debug::log_error("Lua error in action '{}' handler: {}", name, err.what());
                                }
                            });
                        }, "(name: string, type: InputEventType, fn: fun()): integer")
            .method_sig("unbind_action",
                        [](InputComponent& self, const std::string& name, BindingId id) {
                            self.unbind_action(name.c_str(), id);
                        }, "(name: string, id: integer)")
            .method("clear_all_bindings", &InputComponent::clear_all_bindings);

        bind_usertype<Texture>(lua, meta)
            .method("get_width", &Texture::get_width)
            .method("get_height", &Texture::get_height)
            .method("get_path", &Texture::get_path);

        bind_usertype<SpriteComponent>(lua, meta, Bases<Component>{})
            .method("get_texture", &SpriteComponent::get_texture)
            .method_sig("set_texture",
                        [](SpriteComponent& self, const sol::object& value) {
                            if (value.is<TextureRef>()) {
                                self.set_texture(value.as<TextureRef>());
                            }
                            else if (value.is<std::string>()) {
                                self.set_texture(value.as<std::string>());
                            }
                            else {
                                debug::log_error("SpriteComponent:set_texture expects a string path or a Texture");
                            }
                        }, "(path_or_texture: string|Texture)")
            .method("clear_texture", &SpriteComponent::clear_texture)
            .method("get_material", sol::resolve<Material&()>(&SpriteComponent::get_material))
            .method("set_material", &SpriteComponent::set_material, {"material"})
            .method("get_pivot", &SpriteComponent::get_pivot)
            .method("set_pivot", &SpriteComponent::set_pivot, {"pivot"})
            .method("get_scale", &SpriteComponent::get_scale)
            .method("set_scale", &SpriteComponent::set_scale, {"scale"})
            .method("get_z_index", &SpriteComponent::get_z_index)
            .method("set_z_index", &SpriteComponent::set_z_index, {"z_index"})
            .method("get_pixels_per_meter", &SpriteComponent::get_pixels_per_meter)
            .method("set_pixels_per_meter", &SpriteComponent::set_pixels_per_meter, {"value"});

        // shared_ptr lets multiple animators share an instance and keeps the TextureRefs alive.
        Renderer& renderer = m_engine.get_renderer();
        bind_usertype<AnimationClip>(lua, meta)
            .factory_ctor([&renderer](const sol::table& animclip_t) {
                auto clip = std::make_shared<AnimationClip>();
                if (auto textures = animclip_t.get<sol::optional<sol::table>>("textures")) {
                    clip->frames.reserve(textures->size());
                    for (size_t i = 1; i <= textures->size(); ++i) {
                        if (auto path = textures->get<sol::optional<std::string>>(i)) {
                            clip->frames.push_back({renderer.get_or_load_texture(*path)});
                        }
                    }
                }
                clip->fps = animclip_t.get_or("fps", 12.0f);
                clip->looping = animclip_t.get_or("looping", true);
                return clip;
            }, {"config"})
            .method("get_fps", [](const AnimationClip& self) { return self.fps; })
            .method("get_looping", [](const AnimationClip& self) { return self.looping; })
            .method("get_frame_count", [](const AnimationClip& self) {
                return static_cast<int>(self.frames.size());
            });

        bind_factory_schema<AnimationClip>(factory_schemas, "DefineAnimationClip", "AnimationClips",
                                           {"textures", "fps", "looping"});

        bind_usertype<SpriteAnimatorComponent>(lua, meta, Bases<Component>{})
            .method("add_clip", &SpriteAnimatorComponent::add_clip, {"name", "clip"})
            .method("set_default_clip", &SpriteAnimatorComponent::set_default_clip, {"name"})
            .method("get_default_clip", &SpriteAnimatorComponent::get_default_clip)
            .method("get_current_clip", &SpriteAnimatorComponent::get_current_clip)
            .method("get_current_frame", &SpriteAnimatorComponent::get_current_frame)
            .method("play", &SpriteAnimatorComponent::play, {"name"})
            .method("resume", &SpriteAnimatorComponent::resume)
            .method("pause", &SpriteAnimatorComponent::pause)
            .method("stop", &SpriteAnimatorComponent::stop)
            .method("is_playing", &SpriteAnimatorComponent::is_playing)
            .method("set_clips",
                    [](SpriteAnimatorComponent& self, const sol::table& clips_t) {
                        for (auto& kv : clips_t) {
                            if (!kv.first.is<std::string>()) {
                                continue;
                            }
                            const std::string name = kv.first.as<std::string>();
                            auto clip = kv.second.as<sol::optional<std::shared_ptr<AnimationClip>>>();
                            if (clip) {
                                self.add_clip(name, *clip);
                            }
                        }
                    },
                    {"clips"});

        bind_usertype<CameraComponent>(lua, meta, Bases<Component>{})
            .method("get_screen_pixels_per_meter", &CameraComponent::get_screen_pixels_per_meter)
            .method("set_screen_pixels_per_meter", &CameraComponent::set_screen_pixels_per_meter, {"value"})
            .method("get_zoom", &CameraComponent::get_zoom)
            .method("set_zoom", &CameraComponent::set_zoom, {"multiplier"})
            .method("world_to_screen",
                    sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::world_to_screen),
                    {"world_pos"})
            .method("screen_to_world",
                    sol::resolve<Vector2(const Vector2&) const>(&CameraComponent::screen_to_world),
                    {"screen_pos"});

        // Order is load-bearing: Box2D bodies must be attached before colliders.
        bind_component_schema<TransformComponent>(
            schemas, "transform", "get_transform", {
                {"interpolate_physics", "set_interpolate_physics"},
            });

        bind_component_schema<RigidbodyComponent>(
            lua, meta, schemas, "rigidbody", "add_rigidbody", {
                {"body_type", "set_body_type"},
                {"fixed_rotation", "set_fixed_rotation"},
            });

        bind_component_schema<CharacterBodyComponent>(
            lua, meta, schemas, "character_body", "add_character_body", {
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"solver_ignore_mask", "set_solver_ignore_mask"},
                {"capsule", "set_capsule"},
            });

        bind_component_schema<BoxColliderComponent>(
            lua, meta, schemas, "box_collider", "add_box_collider", {
                {"aabb", "set_aabb"},
                {"density", "set_density"},
                {"friction", "set_friction"},
                {"bounciness", "set_bounciness"},
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"trigger", "set_trigger"},
            });

        bind_component_schema<CapsuleColliderComponent>(
            lua, meta, schemas, "capsule_collider", "add_capsule_collider", {
                {"capsule", "set_capsule"},
                {"density", "set_density"},
                {"friction", "set_friction"},
                {"bounciness", "set_bounciness"},
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"trigger", "set_trigger"},
            });

        bind_component_schema<CircleColliderComponent>(
            lua, meta, schemas, "circle_collider", "add_circle_collider", {
                {"circle", "set_circle"},
                {"density", "set_density"},
                {"friction", "set_friction"},
                {"bounciness", "set_bounciness"},
                {"collision_layer", "set_collision_layer"},
                {"collision_mask", "set_collision_mask"},
                {"trigger", "set_trigger"},
            });

        bind_component_schema<SpriteComponent>(
            lua, meta, schemas, "sprite", "add_sprite", {
                {"texture", "set_texture"},
                {"pivot", "set_pivot"},
                {"scale", "set_scale"},
                {"z_index", "set_z_index"},
                {"pixels_per_meter", "set_pixels_per_meter"},
                {"material", "set_material"},
            });

        bind_component_schema<SpriteAnimatorComponent>(
            lua, meta, schemas, "sprite_animator", "add_sprite_animator", {
                {"clips", "set_clips"},
                {"default_clip", "set_default_clip"},
            });

        bind_component_schema<InputComponent>(
            lua, meta, schemas, "input", "add_input", {});

        bind_component_schema<CameraComponent>(
            lua, meta, schemas, "camera", "add_camera", {
                {"screen_pixels_per_meter", "set_screen_pixels_per_meter"},
            });
    }
}
