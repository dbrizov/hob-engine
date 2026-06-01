#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_meta.h"
#include "lua_type_names.h" // IWYU pragma: keep

#include <string>

#include "engine/core/debug.h"
#include "engine/core/logging.h"
#include "engine/core/systems/renderer.h"
#include "engine/math/vector2.h"

namespace hob {
    void LuaScriptSystem::bind_debug() {
        sol::state& m_lua = m_impl->lua;
        LuaMetaRegistry& m_meta = m_impl->meta;

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
            .func_sig("log",
                      [stringify_args](sol::this_state ts, sol::variadic_args args) {
                          debug::log("{}", stringify_args(ts, args));
                      }, "(...: any)")
            .func_sig("log_error",
                      [stringify_args](sol::this_state ts, sol::variadic_args args) {
                          debug::log_error("{}", stringify_args(ts, args));
                      }, "(...: any)")
            .func_sig("draw_line",
                      [](const Vector2& from, const Vector2& to, const Color& color,
                         sol::optional<float> duration, sol::optional<float> thickness) {
                          debug::draw_line(from,
                                           to,
                                           color,
                                           duration.value_or(0.0f),
                                           thickness.value_or(2.0f));
                      }, "(from: Vector2, to: Vector2, color: Color, duration: number?, thickness: number?)")
            .func_sig("draw_circle",
                      [](const Vector2& center, float radius, const Color& color,
                         sol::optional<float> duration, sol::optional<float> thickness, sol::optional<int> segments) {
                          debug::draw_circle(center,
                                             radius,
                                             color,
                                             duration.value_or(0.0f),
                                             thickness.value_or(2.0f),
                                             segments.value_or(16));
                      },
                      "(center: Vector2, radius: number, color: Color, duration: number?, thickness: number?, segments: integer?)");
    }
}
