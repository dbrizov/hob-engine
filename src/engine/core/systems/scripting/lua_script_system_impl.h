#pragma once

#include <sol/sol.hpp>

#include "lua_component_schema.h"
#include "lua_factory_schema.h"
#include "lua_meta.h"
#include "lua_path_schema.h"

namespace hob {
    struct LuaScriptSystemImpl {
        sol::state lua;
        LuaMetaRegistry meta;
        LuaComponentSchemaRegistry component_schemas;
        LuaFactorySchemaRegistry factory_schemas;
        LuaPathSchemaRegistry path_schemas;
    };
}
