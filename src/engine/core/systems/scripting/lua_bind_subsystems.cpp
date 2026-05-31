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

        // Capturing subsystems by reference is safe: Engine's destructor clears all
        // entities before any member destructor runs, so these lambdas can never be
        // invoked through Lua against a half-destroyed subsystem.
        EntitySpawner& spawner = m_engine.get_entity_spawner();
        Input& input = m_engine.get_input();
        Timer& timer = m_engine.get_timer();
        Cursor& cursor = m_engine.get_cursor();
        Physics& physics = m_engine.get_physics();

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

        // EntitySpawner
        bind_table(m_lua, m_meta, "EntitySpawner")
            .func("spawn_entity_c", [&spawner]() { return EntityHandle(spawner.spawn_entity().get_id()); })
            .func("destroy_entity", [&spawner](const EntityHandle& h) { spawner.destroy_entity(h.id); }, {"entity"})
            .func("get_entity", [](EntityId id) { return EntityHandle(id); }, {"id"});

        // Input
        bind_table(m_lua, m_meta, "Input")
            .func("get_mouse_screen_position", [&input]() { return input.get_mouse_screen_position(); });

        // Timer
        bind_table(m_lua, m_meta, "Timer")
            .func("get_fps", [&timer]() { return timer.get_fps(); })
            .func("set_fps", [&timer](uint32_t v) { timer.set_fps(v); }, {"fps"})
            .func("get_time_scale", [&timer]() { return timer.get_time_scale(); })
            .func("set_time_scale", [&timer](float v) { timer.set_time_scale(v); }, {"scale"})
            .func("get_play_time", [&timer]() { return timer.get_play_time(); })
            .func("get_delta_time", [&timer]() { return timer.get_delta_time(); });

        // Cursor
        bind_enum<CursorMode>(m_lua, m_meta, {
                                  {"Default", CursorMode::Default},
                                  {"Confined", CursorMode::Confined},
                              });

        bind_table(m_lua, m_meta, "Cursor")
            .func("has_texture", [&cursor]() { return cursor.has_texture(); })
            .func("set_texture",
                  [&cursor](const std::string& relative_path) { cursor.set_texture(relative_path); },
                  {"relative_path"})
            .func("clear_texture", [&cursor]() { cursor.clear_texture(); })
            .func("get_pivot", [&cursor]() { return cursor.get_pivot(); })
            .func("set_pivot", [&cursor](const Vector2& p) { cursor.set_pivot(p); }, {"pivot"})
            .func("get_scale", [&cursor]() { return cursor.get_scale(); })
            .func("set_scale", [&cursor](const Vector2& s) { cursor.set_scale(s); }, {"scale"})
            .func("get_tint", [&cursor]() { return cursor.get_tint(); })
            .func("set_tint", [&cursor](const Color& c) { cursor.set_tint(c); }, {"tint"})
            .func("is_visible", [&cursor]() { return cursor.is_visible(); })
            .func("set_visible", [&cursor](bool v) { cursor.set_visible(v); }, {"visible"})
            .func("get_mode", [&cursor]() { return cursor.get_mode(); })
            .func("set_mode", [&cursor](CursorMode m) { cursor.set_mode(m); }, {"mode"});

        // Physics
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

        bind_table(m_lua, m_meta, "Camera")
            .func("get_entity", [&spawner]() {
                return EntityHandle(spawner.get_camera_entity()->get_id());
            })
            .func("world_to_screen", [&spawner](const Vector2& world_pos) {
                return spawner.get_camera_entity()->get_component<CameraComponent>()->world_to_screen(world_pos);
            }, {"world_pos"})
            .func("screen_to_world", [&spawner](const Vector2& screen_pos) {
                return spawner.get_camera_entity()->get_component<CameraComponent>()->screen_to_world(screen_pos);
            }, {"screen_pos"})
            .func("get_position", [&spawner]() {
                return spawner.get_camera_entity()->get_transform()->get_position();
            })
            .func("set_position", [&spawner](const Vector2& p) {
                spawner.get_camera_entity()->get_transform()->set_position(p);
            }, {"position"});
    }
}
