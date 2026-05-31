#pragma once

#include <sol/sol.hpp>

#include "lua_meta.h"

namespace hob {
    struct LuaScriptSystemImpl {
        sol::state lua;
        LuaMetaRegistry meta;
    };
}
