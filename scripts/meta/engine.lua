---@meta
-- Hand-written annotations for Lua-side constructs that are NOT bound from
-- C++. C++ bindings are auto-generated into engine.generated.lua by
-- hob::LuaScriptSystem::dump_meta(); do not duplicate them here.

----------------------------------------------------------------------
-- LuaComponent base class
----------------------------------------------------------------------

-- Every component declared with DefineComponent.Foo = { ... } should be
-- annotated as `---@class Foo : LuaComponent` so that self.entity and the
-- lifecycle hooks (init/enter_play/tick/...) are typed correctly.
---@class LuaComponent
---@field entity Entity
---@field class_name string  # Name of the DefineComponent class (set automatically on instantiation).
---@field priority integer?  # Priority execution order for this component type. Defaults to 0 (CP_DEFAULT). Set on the class table (e.g. `Player.priority = -50`), NOT per-instance.
local LuaComponent = {}

--- Called once when the component instance is created (before enter_play).
function LuaComponent:init() end

--- Called when the entity enters play.
function LuaComponent:enter_play() end

--- Called when the entity exits play.
function LuaComponent:exit_play() end

--- Called every frame.
---@param delta_time number
function LuaComponent:tick(delta_time) end

--- Called every fixed physics step. Drive movement here.
---@param fixed_delta_time number
function LuaComponent:physics_tick(fixed_delta_time) end

--- Called every frame after all ticks, intended for debug visualization (Debug.draw_line / Debug.draw_circle).
---@param delta_time number
function LuaComponent:debug_draw_tick(delta_time) end

--- Called when a collider begins colliding with another (solid contact).
---@param other ColliderComponent
function LuaComponent:on_collision_enter(other) end

--- Called when a collider stops colliding with another.
---@param other ColliderComponent
function LuaComponent:on_collision_exit(other) end

--- Called when a collider enters a trigger volume.
---@param other ColliderComponent
function LuaComponent:on_trigger_enter(other) end

--- Called when a collider leaves a trigger volume.
---@param other ColliderComponent
function LuaComponent:on_trigger_exit(other) end

----------------------------------------------------------------------
-- Engine Defs
----------------------------------------------------------------------

--- Assigning `DefineEntity.Foo = { ... }` registers a prefab usable via
--- `EntitySpawner.spawn_entity("Foo", ...)`. Recognized keys: `ticking`,
--- `rigidbody`, `box_collider`, `capsule_collider`, `character_body`,
--- `sprite`, `input`, `lua_components` (list of component class names).
---@class DefineEntity
DefineEntity = {}

--- Assigning `DefineComponent.Foo = { ... }` registers a Lua component class
--- and creates a global `Foo`. The table may contain default fields,
--- and methods (`init`, `enter_play`, `exit_play`, `tick`, ...).
---@class DefineComponent
DefineComponent = {}

--- Assigning `DefineMixin.Foo = { ... }` registers an orthogonal capability
--- bag that can be merged into a Component class via `__mixins = { "Foo" }`.
--- Mixins are NOT components: no lifecycle hooks, no inheritance, no `new`.
--- Keys must not collide with the parent or other mixins; the def's own keys
--- may override a mixin key.
---@class DefineMixin
DefineMixin = {}

--- Assigning `DefineAsset.Foo = "path/under/assets"` registers a named alias
--- for an asset path. Reference it as `Assets.Foo` in prefabs and config;
--- the lookup is deferred and resolved at dispatch time, so DefineAsset calls
--- can live in any file in any load order. When passing `Assets.Foo` directly
--- to a C++ setter, unwrap with `Assets.resolve(...)` or `tostring(...)`.
---@class DefineAsset
DefineAsset = {}

--- Registry of asset aliases declared via `DefineAsset`. `Assets.Foo` returns
--- a deferred reference that resolves to the registered path via __tostring.
---@class Assets
Assets = {}

--- Unwrap an `Assets.X` deferred reference into its underlying path string.
--- Non-asset values are returned unchanged. Use this when passing an asset to
--- a C++ setter that expects a plain string.
---@param value any
---@return any
function Assets.resolve(value) end
