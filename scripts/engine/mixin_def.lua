-- DefineMixin: orthogonal capability bag merged into a Component class.
-- NOT a Component itself: no lifecycle hooks, no inheritance, no `new`.
--
-- Usage:
--   DefineMixin.Damageable = {
--       take_damage = function(self, amount) self.hp = self.hp - amount end,
--       is_dead = function(self) return self.hp <= 0 end,
--   }
--
--   DefineComponent.Player = {
--       __parent = "Character",
--       __mixins = { "Damageable" },
--   }
--
-- Mixin keys must not collide with the parent, prior mixins, or other mixins
-- in the same component; the def's own keys may freely override a mixin
-- (that is an intentional override at the call site).

_G.__mixin_registry = _G.__mixin_registry or {}

_G.DefineMixin = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            log_error("DefineMixin." .. tostring(name) .. " must be assigned a table")
            return
        end
        _G.__mixin_registry[name] = def
        _G[name] = def
    end,
    __index = function(_, name)
        return _G.__mixin_registry[name]
    end,
})
