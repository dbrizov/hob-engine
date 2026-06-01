#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_meta.h"
#include "lua_type_names.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "engine/components/camera_component.h"
#include "engine/components/physics/collider_component.h"
#include "engine/components/transform_component.h"
#include "engine/core/engine.h"
#include "engine/core/systems/cursor.h"
#include "engine/core/systems/input.h"
#include "engine/core/systems/physics.h"
#include "engine/core/systems/timer.h"
#include "engine/entity/entity.h"
#include "engine/entity/entity_spawner.h"

namespace hob {
    void LuaScriptSystem::bind_subsystems() {
        sol::state& m_lua = m_impl->lua;
        LuaMetaRegistry& m_meta = m_impl->meta;

        // Timer
        Timer& timer = m_engine.get_timer();
        bind_table(m_lua, m_meta, "Timer")
            .func("get_fps", [&timer]() { return timer.get_fps(); })
            .func("set_fps", [&timer](uint32_t v) { timer.set_fps(v); }, {"fps"})
            .func("get_time_scale", [&timer]() { return timer.get_time_scale(); })
            .func("set_time_scale", [&timer](float v) { timer.set_time_scale(v); }, {"scale"})
            .func("get_play_time", [&timer]() { return timer.get_play_time(); })
            .func("get_delta_time", [&timer]() { return timer.get_delta_time(); });

        // Input
        Input& input = m_engine.get_input();
        bind_table(m_lua, m_meta, "Input")
            .func("get_mouse_screen_position", [&input]() { return input.get_mouse_screen_position(); });

        // Physics
        Physics& physics = m_engine.get_physics();
        bind_usertype<RaycastHit>(m_lua, m_meta)
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

        bind_table(m_lua, m_meta, "Physics")
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

        // Cursor
        Cursor& cursor = m_engine.get_cursor();
        bind_enum<CursorMode>(m_lua, m_meta, {
                                  {"Default", CursorMode::Default},
                                  {"Confined", CursorMode::Confined},
                              });

        bind_table(m_lua, m_meta, "Cursor")
            .func("has_texture", [&cursor]() { return cursor.has_texture(); })
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

        // EntitySpawner
        EntitySpawner& spawner = m_engine.get_entity_spawner();
        bind_table(m_lua, m_meta, "EntitySpawner")
            .func("spawn_entity_c", [&spawner]() { return EntityHandle(spawner.spawn_entity().get_id()); })
            .func("destroy_entity", [&spawner](const EntityHandle& h) { spawner.destroy_entity(h.id); }, {"entity"})
            .func("get_entity", [](EntityId id) { return EntityHandle(id); }, {"id"});

        // Scripts
        bind_table(m_lua, m_meta, "Scripts")
            .func("run_file", [this](const std::string& relative_path) {
                return run_file(relative_path);
            }, {"relative_path"})
            .func_sig("run_folder", [this](const std::string& relative_path, sol::optional<sol::table> excludes) {
                std::vector<std::string> exclude_list;
                if (excludes) {
                    for (const auto& kv : *excludes) {
                        exclude_list.push_back(kv.second.as<std::string>());
                    }
                }
                return run_folder(relative_path, exclude_list);
            }, "(relative_path: string, excludes: string[]?): boolean");

        // Camera
        Engine& engine = m_engine;
        bind_table(m_lua, m_meta, "Camera")
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
