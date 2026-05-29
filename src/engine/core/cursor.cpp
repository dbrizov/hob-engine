#include "cursor.h"

#include <SDL3/SDL_mouse.h>

#include "app.h"
#include "assets.h"
#include "input.h"
#include "renderer.h"

namespace hob {
    Cursor::Cursor(App& app)
        : m_app(app) {
        SDL_HideCursor();
    }

    Cursor::~Cursor() {
        SDL_ShowCursor();
    }

    TextureId Cursor::get_texture_id() const {
        return m_texture_id;
    }

    void Cursor::set_texture_id(TextureId id) {
        m_texture_id = id;
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

    bool Cursor::is_visible() const {
        return m_visible;
    }

    void Cursor::set_visible(bool visible) {
        m_visible = visible;
    }

    void Cursor::render() {
        if (!m_visible || m_texture_id == INVALID_TEXTURE_ID) {
            return;
        }

        const Assets& assets = m_app.get_assets();
        int texture_width = 0;
        int texture_height = 0;
        assets.get_texture_size(m_texture_id, texture_width, texture_height);

        const float f_w = static_cast<float>(texture_width);
        const float f_h = static_cast<float>(texture_height);
        const Vector2 size(f_w * m_scale.x, f_h * m_scale.y);
        const Vector2 pivot_pixel(size.x * m_pivot.x, size.y * m_pivot.y);

        Vector2 mouse_screen = m_app.get_input().get_mouse_screen_position();
        Vector2 screen_pos(mouse_screen.x - pivot_pixel.x, mouse_screen.y - pivot_pixel.y);

        m_app.get_renderer().draw_sprite(assets.get_texture(m_texture_id),
                                         screen_pos,
                                         size,
                                         pivot_pixel,
                                         0.0f,
                                         m_tint);
    }
}
