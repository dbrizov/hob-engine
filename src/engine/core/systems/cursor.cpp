#include "cursor.h"

#include <SDL3/SDL_mouse.h>

#include "input.h"
#include "renderer.h"
#include "sdl_context.h"

namespace hob {
    Cursor::Cursor(const SdlContext& sdl_context, Renderer& renderer, const Input& input)
        : m_sdl_context(sdl_context)
        , m_renderer(renderer)
        , m_input(input) {
        set_mode(m_mode);
        set_visible(m_is_visible);
    }

    bool Cursor::has_texture() const {
        return m_texture.is_valid();
    }

    void Cursor::set_texture(const std::string& path) {
        m_texture = m_renderer.get_or_load_texture(path);
        set_visible(m_is_visible); // trigger the OS cursor fallback if the load failed
    }

    void Cursor::clear_texture() {
        m_texture.reset();
        set_visible(m_is_visible); // trigger the OS cursor fallback because the texture is gone
    }

    Vector2 Cursor::get_pivot() const {
        return m_pivot;
    }

    void Cursor::set_pivot(const Vector2& pivot) {
        m_pivot = pivot;
    }

    Vector2 Cursor::get_scale() const {
        return m_scale;
    }

    void Cursor::set_scale(const Vector2& scale) {
        m_scale = scale;
    }

    Color Cursor::get_tint() const {
        return m_tint;
    }

    void Cursor::set_tint(const Color& tint) {
        m_tint = tint;
    }

    CursorMode Cursor::get_mode() const {
        return m_mode;
    }

    void Cursor::set_mode(CursorMode mode) {
        if (m_mode == mode) {
            return;
        }

        m_mode = mode;
        SDL_SetWindowMouseGrab(m_sdl_context.get_window(), m_mode == CursorMode::Confined);
    }

    bool Cursor::is_visible() const {
        return m_is_visible;
    }

    void Cursor::set_visible(bool visible) {
        m_is_visible = visible;

        // OS cursor is the fallback when our cursor is on but has no texture;
        // otherwise it stays hidden (the engine owns cursor presentation).
        set_os_cursor_visible(m_is_visible && !m_texture.is_valid());
    }

    bool Cursor::is_os_cursor_visible() const {
        return SDL_CursorVisible();
    }

    void Cursor::set_os_cursor_visible(bool visible) {
        if (visible) {
            SDL_ShowCursor();
        }
        else {
            SDL_HideCursor();
        }
    }

    void Cursor::render() {
        if (!m_is_visible || !m_texture.is_valid() || is_os_cursor_visible()) {
            return;
        }

        const float width = static_cast<float>(m_texture.get_width());
        const float height = static_cast<float>(m_texture.get_height());
        const Vector2 size(width * m_scale.x, height * m_scale.y);
        const Vector2 pivot_pixel(size.x * m_pivot.x, size.y * m_pivot.y);

        Vector2 mouse_screen = m_input.get_mouse_screen_position();
        Vector2 screen_pos(mouse_screen.x - pivot_pixel.x, mouse_screen.y - pivot_pixel.y);

        m_renderer.render_sprite(m_texture.get_id(),
                                 DEFAULT_SPRITE_SHADER_ID,
                                 screen_pos,
                                 size,
                                 pivot_pixel,
                                 0.0f,
                                 m_tint);
    }
}
