---@meta
-- Definition stubs for hob_engine's C++ -> Lua bindings (sol2).
-- Source of truth: src/engine/core/lua_script_system.cpp. Keep in sync when
-- bindings change. This file is annotations-only and is never executed.

----------------------------------------------------------------------
-- Math constants
----------------------------------------------------------------------

---@class Math
---@field PI number
---@field EPSILON number
---@field DEG_TO_RAD number
---@field RAD_TO_DEG number
Math = {}

---@class Numbers
---@field MIN_INT32 integer
---@field MAX_INT32 integer
---@field MAX_UINT32 integer
---@field MIN_FLOAT number
---@field MAX_FLOAT number
Numbers = {}

----------------------------------------------------------------------
-- Vector2
----------------------------------------------------------------------

---@class Vector2
---@field x number
---@field y number
---@operator add(Vector2): Vector2
---@operator sub(Vector2): Vector2
---@operator unm: Vector2
---@operator mul(number): Vector2
---@operator div(number): Vector2
---@overload fun(): Vector2
---@overload fun(x: number, y: number): Vector2
local Vector2 = {}

---@return number
function Vector2:length() end

---@return number
function Vector2:length_sqr() end

---@return Vector2
function Vector2:normalized() end

---@return string
function Vector2:to_string() end

---@return Vector2
function Vector2.zero() end

---@return Vector2
function Vector2.one() end

---@return Vector2
function Vector2.left() end

---@return Vector2
function Vector2.right() end

---@return Vector2
function Vector2.up() end

---@return Vector2
function Vector2.down() end

---@param a Vector2
---@param b Vector2
---@return number
function Vector2.dot(a, b) end

---@param a Vector2
---@param b Vector2
---@return number
function Vector2.distance(a, b) end

---@param a Vector2
---@param b Vector2
---@param t number
---@return Vector2
function Vector2.lerp(a, b, t) end

---@param point Vector2
---@param pivot Vector2
---@param radians number
---@return Vector2
function Vector2.rotate_around(point, pivot, radians) end

_G.Vector2 = Vector2

----------------------------------------------------------------------
-- Capsule / AABB / Color
----------------------------------------------------------------------

-- Capsule
---@class Capsule
---@field center_a Vector2
---@field center_b Vector2
---@field radius number
---@overload fun(center_a: Vector2, center_b: Vector2, radius: number): Capsule
local Capsule = {}

---@return number
function Capsule:get_height() end

_G.Capsule = Capsule

-- AABB
---@class AABB
---@field center Vector2
---@field extents Vector2
---@overload fun(center: Vector2, extents: Vector2): AABB
local AABB = {}

---@return Vector2
function AABB:min() end

---@return Vector2
function AABB:max() end

---@return Vector2
function AABB:size() end

_G.AABB = AABB

-- Color
---@class Color
---@field r number
---@field g number
---@field b number
---@field a number
---@overload fun(): Color
---@overload fun(r: number, g: number, b: number, a: number): Color
local Color = {}

---@return Color
function Color.black() end

---@return Color
function Color.white() end

---@return Color
function Color.gray() end

---@return Color
function Color.red() end

---@return Color
function Color.green() end

---@return Color
function Color.blue() end

---@return Color
function Color.yellow() end

---@return Color
function Color.magenta() end

---@return Color
function Color.cyan() end

---@return Color
function Color.orange() end

_G.Color = Color

----------------------------------------------------------------------
-- Enums
----------------------------------------------------------------------

---@class BodyType
---@field Static integer
---@field Dynamic integer
---@field Kinematic integer
BodyType = {}

---@class InputEventType
---@field Axis integer
---@field Pressed integer
---@field Released integer
InputEventType = {}

----------------------------------------------------------------------
-- Entity & Components
----------------------------------------------------------------------

-- LuaComponent base class
--
-- Every component declared with DefineComponent.Foo = { ... } should be
-- annotated as `---@class Foo : LuaComponent` so that self.entity and the
-- lifecycle hooks (init/enter_play/tick/...) are typed correctly.
---@class LuaComponent
---@field entity Entity
---@field priority integer?  # Priority execution order for this component type. Defaults to 0 (CP_DEFAULT). Set on the class table (e.g. `Player.priority = -50`), NOT per-instance.
local LuaComponent = {}

--- Called once when the component instance is created (before enter_play).
function LuaComponent:init() end

--- Called when the entity enters play. Bind input here.
function LuaComponent:enter_play() end

--- Called when the entity exits play. Unbind input here.
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

-- Component
---@class Component
local Component = {}

---@return Entity
function Component:get_entity() end

-- TransformComponent
---@class TransformComponent : Component
local TransformComponent = {}

---@return Vector2
function TransformComponent:get_position() end

---@param position Vector2
function TransformComponent:set_position(position) end

---@return number
function TransformComponent:get_rotation() end

---@param radians number
function TransformComponent:set_rotation(radians) end

---@return Vector2
function TransformComponent:get_scale() end

---@param scale Vector2
function TransformComponent:set_scale(scale) end

-- SpriteComponent
---@class SpriteComponent : Component
local SpriteComponent = {}

---@return integer
function SpriteComponent:get_texture_id() end

---@param id integer
function SpriteComponent:set_texture_id(id) end

---@return Vector2
function SpriteComponent:get_pivot() end

---@param pivot Vector2
function SpriteComponent:set_pivot(pivot) end

---@return Vector2
function SpriteComponent:get_scale() end

---@param scale Vector2
function SpriteComponent:set_scale(scale) end

---@return Color
function SpriteComponent:get_tint() end

---@param color Color
function SpriteComponent:set_tint(color) end

---@return integer
function SpriteComponent:get_z_index() end

---@param z_index integer
function SpriteComponent:set_z_index(z_index) end

-- CameraComponent
---@class CameraComponent : Component
local CameraComponent = {}

---@param world_pos Vector2
---@return Vector2
function CameraComponent:world_to_screen(world_pos) end

---@param screen_pos Vector2
---@return Vector2
function CameraComponent:screen_to_world(screen_pos) end

-- RigidbodyComponent
---@class RigidbodyComponent : Component
local RigidbodyComponent = {}

---@return integer
function RigidbodyComponent:get_body_type() end

---@param type integer  # BodyType.Static, BodyType.Kinematic...
function RigidbodyComponent:set_body_type(type) end

---@return boolean
function RigidbodyComponent:has_fixed_rotation() end

---@param is_fixed boolean
function RigidbodyComponent:set_fixed_rotation(is_fixed) end

---@return Vector2
function RigidbodyComponent:get_velocity() end

---@param velocity Vector2
function RigidbodyComponent:set_velocity(velocity) end

---@return Vector2
function RigidbodyComponent:get_position() end

---@param position Vector2
function RigidbodyComponent:set_position(position) end

---@return number
function RigidbodyComponent:get_rotation() end

---@param radians number
function RigidbodyComponent:set_rotation(radians) end

-- ColliderComponent
---@class ColliderComponent : Component
local ColliderComponent = {}

---@return number
function ColliderComponent:get_density() end

---@param density number
function ColliderComponent:set_density(density) end

---@return number
function ColliderComponent:get_friction() end

---@param friction number
function ColliderComponent:set_friction(friction) end

---@return number
function ColliderComponent:get_bounciness() end

---@param bounciness number
function ColliderComponent:set_bounciness(bounciness) end

---@return integer
function ColliderComponent:get_collision_layer() end

---@param layer integer
function ColliderComponent:set_collision_layer(layer) end

---@return integer
function ColliderComponent:get_collision_mask() end

---@param mask integer
function ColliderComponent:set_collision_mask(mask) end

---@return boolean
function ColliderComponent:is_trigger() end

---@param trigger boolean
function ColliderComponent:set_trigger(trigger) end

-- BoxColliderComponent
---@class BoxColliderComponent : ColliderComponent
local BoxColliderComponent = {}

---@return AABB
function BoxColliderComponent:get_aabb() end

---@param aabb AABB
function BoxColliderComponent:set_aabb(aabb) end

-- CapsuleColliderComponent
---@class CapsuleColliderComponent : ColliderComponent
local CapsuleColliderComponent = {}

---@return Capsule
function CapsuleColliderComponent:get_capsule() end

---@param capsule Capsule
function CapsuleColliderComponent:set_capsule(capsule) end

-- CharacterBodyComponent
---@class CharacterBodyComponent : Component
local CharacterBodyComponent = {}

---@return integer
function CharacterBodyComponent:get_collision_layer() end

---@param layer integer
function CharacterBodyComponent:set_collision_layer(layer) end

---@return integer
function CharacterBodyComponent:get_collision_mask() end

---@param mask integer
function CharacterBodyComponent:set_collision_mask(mask) end

---@return integer
function CharacterBodyComponent:get_solver_ignore_mask() end

---@param mask integer
function CharacterBodyComponent:set_solver_ignore_mask(mask) end

---@param desired_velocity Vector2
---@param fixed_dt number
function CharacterBodyComponent:move_and_slide(desired_velocity, fixed_dt) end

---@return Vector2
function CharacterBodyComponent:get_velocity() end

---@param velocity Vector2
function CharacterBodyComponent:set_velocity(velocity) end

---@return Vector2
function CharacterBodyComponent:get_position() end

---@param position Vector2
function CharacterBodyComponent:set_position(position) end

---@return number
function CharacterBodyComponent:get_rotation() end

---@param radians number
function CharacterBodyComponent:set_rotation(radians) end

-- InputComponent
---@class InputComponent : Component
local InputComponent = {}

---@param name string
---@param fn fun(value: number)
---@return integer binding_id
function InputComponent:bind_axis(name, fn) end

---@param name string
---@param id integer
function InputComponent:unbind_axis(name, id) end

---@param name string
---@param event_type integer  # InputEventType.Pressed or InputEventType.Released
---@param fn fun()
---@return integer binding_id
function InputComponent:bind_action(name, event_type, fn) end

---@param name string
---@param id integer
function InputComponent:unbind_action(name, id) end

function InputComponent:clear_all_bindings() end

-- LuaScriptComponent
---@class LuaScriptComponent : Component
local LuaScriptComponent = {}

---@return string
function LuaScriptComponent:get_class_name() end

-- Entity
---@class Entity
local Entity = {}

---@return integer
function Entity:get_id() end

---@return boolean
function Entity:is_in_play() end

---@return boolean
function Entity:is_ticking() end

---@param ticking boolean
function Entity:set_ticking(ticking) end

---@return RigidbodyComponent
function Entity:add_rigidbody() end

---@return BoxColliderComponent
function Entity:add_box_collider() end

---@return CapsuleColliderComponent
function Entity:add_capsule_collider() end

---@return CharacterBodyComponent
function Entity:add_character_body() end

---@return SpriteComponent
function Entity:add_sprite() end

---@return InputComponent
function Entity:add_input() end

---@param class_name string
---@return LuaScriptComponent
function Entity:add_lua_component(class_name) end

---@return TransformComponent
function Entity:get_transform() end

---@return RigidbodyComponent|nil
function Entity:get_rigidbody() end

---@return BoxColliderComponent|nil
function Entity:get_box_collider() end

---@return CapsuleColliderComponent|nil
function Entity:get_capsule_collider() end

---@return CharacterBodyComponent|nil
function Entity:get_character_body() end

---@return SpriteComponent|nil
function Entity:get_sprite() end

---@return InputComponent|nil
function Entity:get_input() end

---@param class_name string
---@return LuaScriptComponent|nil
function Entity:get_lua_component(class_name) end

---@return LuaScriptComponent[]
function Entity:get_lua_components() end

----------------------------------------------------------------------
-- Subsystem tables
----------------------------------------------------------------------

-- EntitySpawner
---@class EntitySpawner
EntitySpawner = {}

--- Raw C++ spawn — bare entity with only a TransformComponent.
---@return Entity
function EntitySpawner.spawn_entity_c() end

---@param id integer
function EntitySpawner.destroy_entity(id) end

---@param id integer
---@return Entity|nil
function EntitySpawner.get_entity(id) end

-- Input
---@class Input
Input = {}

---@return Vector2
function Input.get_mouse_screen_position() end

-- Timer
---@class Timer
Timer = {}

---@return integer
function Timer.get_fps() end

---@param fps integer
function Timer.set_fps(fps) end

---@return number
function Timer.get_time_scale() end

---@param scale number
function Timer.set_time_scale(scale) end

---@return number
function Timer.get_play_time() end

---@return number
function Timer.get_delta_time() end

-- Assets
---@class Assets
Assets = {}

---@param relative_path string
---@return integer texture_id
function Assets.load_texture(relative_path) end

-- Camera
---@class Camera
Camera = {}

---@return Entity
function Camera.get_entity() end

---@param world_pos Vector2
---@return Vector2
function Camera.world_to_screen(world_pos) end

---@param screen_pos Vector2
---@return Vector2
function Camera.screen_to_world(screen_pos) end

---@return Vector2
function Camera.get_position() end

---@param p Vector2
function Camera.set_position(p) end

----------------------------------------------------------------------
-- Logging
----------------------------------------------------------------------

---@param ... any
function log(...) end

---@param ... any
function log_error(...) end

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
