#pragma once

#include "assets.h"
#include "renderer.h"
#include "engine/math/vector2.h"

namespace hob {
    class App;

    enum class CursorMode {
        Default, // Free movement; cursor can leave the window
        Confined, // Cursor is grabbed inside the window bounds
    };

    // Owns the custom in-game cursor. Hides the OS cursor at construction.
    // Eliminates lag between the engine's mouse position and the OS' mouse position.
    class Cursor {
        App& m_app;
        TextureId m_texture_id = INVALID_TEXTURE_ID;
        Vector2 m_pivot = Vector2(0.5f, 0.5f);
        Vector2 m_scale = Vector2(1.0f, 1.0f);
        Color m_tint = Color::white();
        CursorMode m_mode = CursorMode::Default;
        bool m_is_visible = true;

    public:
        explicit Cursor(App& app);

        Cursor(const Cursor&) = delete;
        Cursor& operator=(const Cursor&) = delete;

        TextureId get_texture_id() const;
        void set_texture_id(TextureId id);

        Vector2 get_pivot() const;
        void set_pivot(const Vector2& pivot);

        Vector2 get_scale() const;
        void set_scale(const Vector2& scale);

        Color get_tint() const;
        void set_tint(const Color& tint);

        CursorMode get_mode() const;
        void set_mode(CursorMode mode);

        bool is_visible() const;
        void set_visible(bool visible);

        bool is_os_cursor_visible() const;
        void set_os_cursor_visible(bool visible);

        void render();
    };
}
