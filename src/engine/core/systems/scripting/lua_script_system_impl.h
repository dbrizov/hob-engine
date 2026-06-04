#pragma once

#include <sol/sol.hpp>

#include "lua_meta.h"
#include "lua_schema_component.h"
#include "lua_schema_factory.h"
#include "lua_schema_path.h"

namespace hob {
    struct LuaScriptSystemImpl {
        sol::state lua;
        LuaMetaRegistry meta;
        LuaComponentSchemaRegistry component_schemas;
        LuaFactorySchemaRegistry factory_schemas;
        LuaPathSchemaRegistry path_schemas;
    };
}
