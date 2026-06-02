-- DefineAnimationClip: Named alias for an AnimationClip definition.
--
-- Usage:
--   DefineAnimationClip.PlayerRun = {
--       textures = { Assets.Run01, Assets.Run02, Assets.Run03 },
--       fps = 12,
--       looping = true,
--   }
--
-- Then in a prefab:
--   sprite_animator = {
--       clips = { idle = AnimationClips.PlayerIdle, run = AnimationClips.PlayerRun },
--       default_clip = "idle",
--   }
--
-- `AnimationClips.Name` returns a deferred reference. The actual C++ AnimationClip is
-- constructed lazily on first resolve (and cached). DefineAnimationClip calls can live
-- in any file in any load order, same contract as DefineAsset / DefineEntity.
--
-- The Lua side is the registry of *named* clips; the C++ side provides the AnimationClip
-- type and factory only. No C++ code calls back into Lua.

_G.__animation_clip_defs = _G.__animation_clip_defs or {}
_G.__animation_clips_built = _G.__animation_clips_built or {}

local function build_clip(name)
    local def = _G.__animation_clip_defs[name]
    if not def then
        Debug.log_error("AnimationClip '" .. name .. "' is not defined")
        return nil
    end

    local resolved_textures = {}
    if def.textures then
        for i, t in ipairs(def.textures) do
            resolved_textures[i] = Assets.resolve(t)
        end
    end

    local clip = AnimationClip {
        textures = resolved_textures,
        fps = def.fps,
        looping = def.looping,
    }

    _G.__animation_clips_built[name] = clip
    return clip
end

local clip_ref_mt = {
    __tostring = function(self)
        return "AnimationClip(" .. tostring(self.__clip) .. ")"
    end,
}

_G.DefineAnimationClip = setmetatable({}, {
    __newindex = function(_, name, def)
        if type(def) ~= "table" then
            Debug.log_error("DefineAnimationClip." .. tostring(name) .. " must be assigned a table")
            return
        end

        _G.__animation_clip_defs[name] = def
    end,
})

_G.AnimationClips = setmetatable({}, {
    __index = function(t, name)
        local wrapper = setmetatable({ __clip = name }, clip_ref_mt)
        rawset(t, name, wrapper)
        return wrapper
    end,
})

rawset(_G.AnimationClips, "resolve", function(value)
    if type(value) == "table" and getmetatable(value) == clip_ref_mt then
        local name = value.__clip
        local built = _G.__animation_clips_built[name]
        if built then
            return built
        end
        return build_clip(name)
    end

    return value
end)
