#pragma once

#include <sol/sol.hpp>

namespace hob {
    struct LuaScriptComponentImpl {
        sol::table lua_instance;
    };
}
