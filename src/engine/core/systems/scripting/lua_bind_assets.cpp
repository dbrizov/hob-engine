#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_path_schema.h"

namespace hob {
    void LuaScriptSystem::bind_assets() {
        LuaPathSchemaRegistry& path_schemas = m_impl->path_schemas;

        bind_path_schema(path_schemas, "DefineAsset", "Assets", "Asset");
        bind_path_schema(path_schemas, "DefineTexture", "Textures", "Texture");
        bind_path_schema(path_schemas, "DefineShader", "Shaders", "Shader");
    }
}
