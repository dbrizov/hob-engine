#pragma once

#include <sol/sol.hpp>

#include "lua_component_schema.h"
#include "lua_factory_schema.h"
#include "lua_meta.h"

namespace hob {
    struct LuaScriptSystemImpl {
        sol::state lua;
        LuaMetaRegistry meta;
        LuaComponentSchemaRegistry component_schemas;
        LuaFactorySchemaRegistry factory_schemas;
    };
}
