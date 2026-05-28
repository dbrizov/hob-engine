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
-- DefineComponent / DefineEntity (Lua-side, scripts/engine/*.lua)
----------------------------------------------------------------------

--- Assigning `DefineComponent.Foo = { ... }` registers a Lua component class
--- and creates a global `Foo`. The table may contain default fields,
--- methods (`init`, `enter_play`, `exit_play`, `tick`, ...), and an
--- optional `__parents = { "Base", ... }` list for inheritance.
---@class DefineComponent
DefineComponent = {}

--- Assigning `DefineEntity.Foo = { ... }` registers a prefab usable via
--- `EntitySpawner.spawn_entity("Foo", ...)`. Recognized keys: `ticking`,
--- `rigidbody`, `box_collider`, `capsule_collider`, `character_body`,
--- `sprite`, `input`, `lua_components` (list of component class names).
---@class DefineEntity
DefineEntity = {}
