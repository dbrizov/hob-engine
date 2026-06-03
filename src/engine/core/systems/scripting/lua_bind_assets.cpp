#include "lua_script_system.h"
#include "lua_script_system_impl.h"
#include "lua_path_schema.h"

namespace hob {
    // Typed path-alias namespaces. Each entry registers `DefineX.Name = "path"` on the Lua
    // side, producing a deferred ref under `Registry.Name` that unwraps to the path string.
    // Adding a new namespace here is the only change needed — Lua picks it up via the
    // generated path_schemas.generated.lua on next run.
    void LuaScriptSystem::bind_assets() {
        LuaPathSchemaRegistry& path_schemas = m_impl->path_schemas;

        bind_path_schema(path_schemas, "Assets", "DefineAsset", "Asset");
        bind_path_schema(path_schemas, "Textures", "DefineTexture", "Texture");
        bind_path_schema(path_schemas, "Shaders", "DefineShader", "Shader");
    }
}
