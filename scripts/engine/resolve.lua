-- Unwraps any deferred DefineX reference (Assets.X, Materials.X, AnimationClips.X, ...)
-- and recurses into plain tables so nested fields are resolved too. Plain values
-- (numbers, strings, usertypes, wrappers without __resolve) pass through.
--
-- Protocol: a "deferred reference" is any table whose metatable carries a
-- `__resolve(self)` function. That function returns the unwrapped value. This is
-- the only API any dispatch site needs; registries don't expose per-type
-- `.resolve` methods.

function _G.resolve_asset(value)
    local mt = getmetatable(value)
    if mt and mt.__resolve then
        return mt.__resolve(value)
    end

    if type(value) == "table" and mt == nil then
        local out = {}
        for k, v in pairs(value) do
            out[k] = resolve_asset(v)
        end
        return out
    end

    return value
end
