#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_path_schema.h"

namespace hob {
    void LuaScriptSystem::bind_assets() {
        LuaPathSchemaRegistry& path_schemas = m_impl->path_schemas;

        bind_path_schema(path_schemas, "Assets", "DefineAsset", "Asset");
        bind_path_schema(path_schemas, "Textures", "DefineTexture", "Texture");
        bind_path_schema(path_schemas, "Shaders", "DefineShader", "Shader");
    }
}
