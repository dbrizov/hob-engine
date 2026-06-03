#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_factory_schema.h"
#include "lua_meta.h"
#include "lua_type_names.h" // IWYU pragma: keep

#include <functional>
#include <string>
#include <vector>

#include "engine/components/camera_component.h"
#include "engine/components/physics/collider_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/cursor.h"
#include "engine/core/systems/input.h"
#include "engine/core/systems/physics.h"
#include "engine/core/systems/renderer.h"
#include "engine/core/systems/timer.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    namespace {
        void bind_renderer(sol::state& lua,
                           LuaMetaRegistry& meta,
                           LuaFactorySchemaRegistry& factory_schemas,
                           Renderer& renderer) {
            bind_usertype<Material>(lua, meta)
                .factory_ctor([&renderer](sol::table mat_t) {
                    Material mat;
                    sol::object sh_obj = mat_t["shader"];
                    if (sh_obj.valid() && sh_obj.get_type() != sol::type::lua_nil) {
                        sol::state_view sv(mat_t.lua_state());
                        std::string path = sv["unwrap_def"](sh_obj);
                        mat.shader_id = renderer.get_or_build_sprite_shader(path);
                    }
                    if (auto tint = mat_t.get<sol::optional<Color>>("tint")) {
                        mat.tint = *tint;
                    }
                    return mat;
                }, {"config"})
                .method("get_tint", [](const Material& self) { return self.tint; })
                .method("set_tint", [](Material& self, const Color& tint) { self.tint = tint; }, {"tint"})
                .method("set_shader",
                        [&renderer](Material& self, const std::string& path) {
                            self.shader_id = renderer.get_or_build_sprite_shader(path);
                        },
                        {"path"});

            bind_factory_schema<Material>(factory_schemas, "DefineMaterial", "Materials",
                                          {"shader", "tint"});
        }

        void bind_timer(sol::state& lua, LuaMetaRegistry& meta, Timer& timer) {
            bind_table(lua, meta, "Timer")
                .func("get_fps", [&timer]() { return timer.get_fps(); })
                .func("set_fps", [&timer](uint32_t v) { timer.set_fps(v); }, {"fps"})
                .func("get_time_scale", [&timer]() { return timer.get_time_scale(); })
                .func("set_time_scale", [&timer](float v) { timer.set_time_scale(v); }, {"scale"})
                .func("get_play_time", [&timer]() { return timer.get_play_time(); })
                .func("get_delta_time", [&timer]() { return timer.get_delta_time(); });
        }

        void bind_input(sol::state& lua, LuaMetaRegistry& meta, Input& input) {
            bind_table(lua, meta, "Input")
                .func("get_mouse_screen_position", [&input]() { return input.get_mouse_screen_position(); });
        }

        void bind_physics(sol::state& lua, LuaMetaRegistry& meta, Physics& physics) {
            bind_usertype<RaycastHit>(lua, meta)
                .field("collider", &RaycastHit::collider)
                .field("point", &RaycastHit::point)
                .field("normal", &RaycastHit::normal)
                .field("distance", &RaycastHit::distance)
                .field("hit", &RaycastHit::hit)
                .property_sig("entity", [](const RaycastHit& h, sol::this_state ts) -> sol::object {
                    sol::state_view sv(ts);
                    if (h.collider == nullptr) {
                        return sol::lua_nil;
                    }

                    return sol::make_object(sv, EntityHandle(h.collider->get_entity().get_id()));
                }, "Entity?");

            bind_table(lua, meta, "Physics")
                .func("raycast",
                      [&physics](const Vector2& origin, const Vector2& direction, float distance,
                                 sol::optional<uint64_t> layer_mask) {
                          return physics.raycast(origin, direction, distance, layer_mask.value_or(~0ull));
                      },
                      {"origin", "direction", "distance", "layer_mask"})
                .func("raycast_all",
                      [&physics](const Vector2& origin, const Vector2& direction, float distance,
                                 sol::optional<uint64_t> layer_mask) {
                          return physics.raycast_all(origin, direction, distance, layer_mask.value_or(~0ull));
                      },
                      {"origin", "direction", "distance", "layer_mask"});
        }

        void bind_cursor(sol::state& lua, LuaMetaRegistry& meta, Cursor& cursor) {
            bind_enum<CursorMode>(lua, meta, {
                                      {"Default", CursorMode::Default},
                                      {"Confined", CursorMode::Confined},
                                  });

            bind_table(lua, meta, "Cursor")
                .func("get_texture", [&cursor]() -> const TextureRef& { return cursor.get_texture(); })
                .func("set_texture", [&cursor](const std::string& path) { cursor.set_texture(path); }, {"path"})
                .func("clear_texture", [&cursor]() { cursor.clear_texture(); })
                .func("get_pivot", [&cursor]() { return cursor.get_pivot(); })
                .func("set_pivot", [&cursor](const Vector2& p) { cursor.set_pivot(p); }, {"pivot"})
                .func("get_scale", [&cursor]() { return cursor.get_scale(); })
                .func("set_scale", [&cursor](const Vector2& s) { cursor.set_scale(s); }, {"scale"})
                .func("get_material", [&cursor]() -> Material& { return cursor.get_material(); })
                .func("set_material", [&cursor](const Material& m) { cursor.set_material(m); }, {"material"})
                .func("is_visible", [&cursor]() { return cursor.is_visible(); })
                .func("set_visible", [&cursor](bool v) { cursor.set_visible(v); }, {"visible"})
                .func("get_mode", [&cursor]() { return cursor.get_mode(); })
                .func("set_mode", [&cursor](CursorMode m) { cursor.set_mode(m); }, {"mode"});
        }

        void bind_entity_spawner(sol::state& lua, LuaMetaRegistry& meta, EntitySpawner& spawner) {
            bind_table(lua, meta, "EntitySpawner")
                .func("spawn_entity_c", [&spawner]() { return EntityHandle(spawner.spawn_entity().get_id()); })
                .func("destroy_entity", [&spawner](const EntityHandle& h) { spawner.destroy_entity(h.id); }, {"entity"})
                .func("get_entity", [](EntityId id) { return EntityHandle(id); }, {"id"});
        }

        void bind_scripts(sol::state& lua,
                          LuaMetaRegistry& meta,
                          std::function<bool(const std::string&)> run_file,
                          std::function<bool(const std::string&, const std::vector<std::string>&)> run_folder) {
            bind_table(lua, meta, "Scripts")
                .func("run_file", [run_file = std::move(run_file)](const std::string& relative_path) {
                    return run_file(relative_path);
                }, {"relative_path"})
                .func_sig("run_folder",
                          [run_folder = std::move(run_folder)](const std::string& relative_path,
                                                               sol::optional<sol::table> excludes) {
                              std::vector<std::string> exclude_list;
                              if (excludes) {
                                  for (const auto& kv : *excludes) {
                                      exclude_list.push_back(kv.second.as<std::string>());
                                  }
                              }
                              return run_folder(relative_path, exclude_list);
                          }, "(relative_path: string, excludes: string[]?): boolean");
        }

        void bind_camera(sol::state& lua, LuaMetaRegistry& meta, Engine& engine) {
            bind_table(lua, meta, "Camera")
                .func_sig("get_active", [&engine](sol::this_state ts) -> sol::object {
                    CameraComponent* cam = engine.get_active_camera();
                    if (cam == nullptr) {
                        return sol::lua_nil;
                    }
                    return sol::make_object(sol::state_view(ts), EntityHandle(cam->get_entity().get_id()));
                }, "(): Entity?")
                .func("world_to_screen", [&engine](const Vector2& world_pos) {
                    CameraComponent* cam = engine.get_active_camera();
                    return cam ? cam->world_to_screen(world_pos) : Vector2();
                }, {"world_pos"})
                .func("screen_to_world", [&engine](const Vector2& screen_pos) {
                    CameraComponent* cam = engine.get_active_camera();
                    return cam ? cam->screen_to_world(screen_pos) : Vector2();
                }, {"screen_pos"})
                .func("get_position", [&engine]() {
                    CameraComponent* cam = engine.get_active_camera();
                    return cam ? cam->get_entity().get_transform()->get_position() : Vector2();
                })
                .func("set_position", [&engine](const Vector2& p) {
                    CameraComponent* cam = engine.get_active_camera();
                    if (cam != nullptr) {
                        cam->get_entity().get_transform()->set_position(p);
                    }
                }, {"position"})
                .func("get_zoom", [&engine]() {
                    CameraComponent* cam = engine.get_active_camera();
                    return cam ? cam->get_zoom() : 1.0f;
                })
                .func("set_zoom", [&engine](float multiplier) {
                    CameraComponent* cam = engine.get_active_camera();
                    if (cam != nullptr) {
                        cam->set_zoom(multiplier);
                    }
                }, {"multiplier"});
        }
    }

    void LuaScriptSystem::bind_systems() {
        sol::state& lua = m_impl->lua;
        LuaMetaRegistry& meta = m_impl->meta;
        LuaFactorySchemaRegistry& factory_schemas = m_impl->factory_schemas;

        bind_renderer(lua, meta, factory_schemas, m_engine.get_renderer());
        bind_timer(lua, meta, m_engine.get_timer());
        bind_input(lua, meta, m_engine.get_input());
        bind_physics(lua, meta, m_engine.get_physics());
        bind_cursor(lua, meta, m_engine.get_cursor());
        bind_entity_spawner(lua, meta, m_engine.get_entity_spawner());
        bind_scripts(lua, meta,
                     [this](const std::string& p) { return run_file(p); },
                     [this](const std::string& p, const std::vector<std::string>& ex) { return run_folder(p, ex); });
        bind_camera(lua, meta, m_engine);
    }
}
