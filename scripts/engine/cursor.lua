-- Data-driven cursor configuration.
--
-- Usage:
--   Cursor.config {
--       texture = "images/cursor.png",   -- path under assets/
--       pivot   = Vector2(0.0, 0.0),     -- normalized (0,0)=top-left, (0.5,0.5)=center
--       scale   = Vector2(1.0, 1.0),
--       tint    = Color.white(),
--       visible = true,
--   }
--
-- All fields are optional; unspecified ones keep their current value.

---@class CursorConfig
---@field texture string?
---@field pivot Vector2?
---@field scale Vector2?
---@field tint Color?
---@field visible boolean?

---@param config CursorConfig
function Cursor.config(config)
    if config.texture then
        Cursor.set_texture(config.texture)
    end

    if config.pivot then
        Cursor.set_pivot(config.pivot)
    end

    if config.scale then
        Cursor.set_scale(config.scale)
    end

    if config.tint then
        Cursor.set_tint(config.tint)
    end

    if config.visible ~= nil then
        Cursor.set_visible(config.visible)
    end
end
