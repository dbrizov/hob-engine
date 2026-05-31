#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_meta.h"
#include "lua_type_names.h" // IWYU pragma: keep

#include <format>
#include <string>

#include "engine/core/engine.h"
#include "engine/components/lua_script_component.h"
#include "engine/components/lua_script_component_impl.h"
#include "engine/components/input_component.h"
#include "engine/components/sprite_component.h"
#include "engine/components/transform_component.h"
#include "engine/components/physics/box_collider_component.h"
#include "engine/components/physics/capsule_collider_component.h"
#include "engine/components/physics/character_body_component.h"
#include "engine/components/physics/circle_collider_component.h"
#include "engine/components/physics/rigidbody_component.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    void LuaScriptSystem::bind_entity() {
        sol::state& m_lua = m_impl->lua;
        LuaMetaRegistry& m_meta = m_impl->meta;

        const EntitySpawner& spawner = m_engine.get_entity_spawner();

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
            .method("add_circle_collider", [get_entity](const EntityHandle& h) -> CircleColliderComponent* {
                Entity* e = get_entity(h);
                return e ? e->add_component<CircleColliderComponent>() : nullptr;
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

                    return lua_comp->impl().lua_instance;
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
            .method("get_circle_collider", [get_entity](const EntityHandle& h) -> CircleColliderComponent* {
                Entity* e = get_entity(h);
                return e ? e->get_component<CircleColliderComponent>() : nullptr;
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
                                    return lua_comp->impl().lua_instance;
                                }
                            }

                            return sol::lua_nil;
                        }, "(class_name: string): LuaComponent?")
            .method_sig("get_lua_components",
                        [this, get_entity](const EntityHandle& h) {
                            sol::table out = m_impl->lua.create_table();
                            Entity* e = get_entity(h);
                            if (e == nullptr) {
                                return out;
                            }

                            for (LuaScriptComponent* lua_comp : e->get_components<LuaScriptComponent>()) {
                                out.add(lua_comp->impl().lua_instance);
                            }

                            return out;
                        }, "(): LuaComponent[]")
            .op_tostring([get_entity](const EntityHandle& h) {
                Entity* e = get_entity(h);
                return e ? e->to_string() : std::format("Entity(invalid, id = {})", h.id);
            });
    }
}
