#pragma once

#include <sol/sol.hpp>

namespace hob {
    struct LuaScriptComponentImpl {
        sol::table lua_instance;

        sol::protected_function init;
        sol::protected_function enter_play;
        sol::protected_function exit_play;
        sol::protected_function tick;
        sol::protected_function physics_tick;
        sol::protected_function late_tick;
        sol::protected_function debug_draw_tick;
        sol::protected_function on_collision_enter;
        sol::protected_function on_collision_exit;
        sol::protected_function on_trigger_enter;
        sol::protected_function on_trigger_exit;
    };
} // namespace hob
