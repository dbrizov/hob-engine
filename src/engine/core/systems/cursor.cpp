#include "cursor.h"

#include <SDL3/SDL_mouse.h>

#include "input.h"
#include "renderer/renderer.h"
#include "sdl_context.h"

namespace hob {
    Cursor::Cursor(const SdlContext& sdl_context, Renderer& renderer, const Input& input)
        : m_sdl_context(sdl_context)
        , m_renderer(renderer)
        , m_input(input) {
        set_mode(m_mode);
        set_visible(m_is_visible);
    }

    const TextureRef& Cursor::get_texture() const {
        return m_texture;
    }

    void Cursor::set_texture(TextureRef texture) {
        m_texture = std::move(texture);
        set_visible(m_is_visible); // trigger the OS cursor fallback if the texture is null
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

    const Material& Cursor::get_material() const {
        return m_material;
    }

    Material& Cursor::get_material() {
        return m_material;
    }

    void Cursor::set_material(const Material& material) {
        m_material = material;
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
        set_os_cursor_visible(m_is_visible && m_texture == nullptr);
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

    void Cursor::draw() {
        if (m_texture == nullptr || !m_is_visible || is_os_cursor_visible()) {
            return;
        }

        const float width = static_cast<float>(m_texture->get_width());
        const float height = static_cast<float>(m_texture->get_height());

        // The FBO is stretched to the window, so counter-scale the cursor by the window-to-logical
        // ratio to keep it a constant pixel size on screen regardless of window size.
        // Multiplying by dpi_scale keeps it at a constant physical size on hi-DPI displays.
        const Vector2 window_size = m_sdl_context.get_window_size();
        const Vector2 logical_size = m_renderer.get_logical_size();
        const float dpi_scale = m_sdl_context.get_dpi_scale();
        const float scale_factor_x = (window_size.x > 0.0f) ? (logical_size.x / window_size.x) * dpi_scale : 1.0f;
        const float scale_factor_y = (window_size.y > 0.0f) ? (logical_size.y / window_size.y) * dpi_scale : 1.0f;

        const Vector2 size(width * m_scale.x * scale_factor_x, height * m_scale.y * scale_factor_y);
        const Vector2 pivot(size.x * m_pivot.x, size.y * m_pivot.y);
        const float rotation_rad = 0.0f;
        Vector2 mouse_screen = m_input.get_mouse_screen_position();
        Vector2 screen_pos(mouse_screen.x - pivot.x, mouse_screen.y - pivot.y);

        m_renderer.draw_overlay_sprite(m_texture, screen_pos, size, pivot, rotation_rad, m_material);
    }
}
