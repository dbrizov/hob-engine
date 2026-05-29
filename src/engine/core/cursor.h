#pragma once

#include "assets.h"
#include "renderer.h"
#include "engine/math/vector2.h"

namespace hob {
    class App;

    // Owns the custom in-game cursor. Hides the OS cursor at construction
    // so the in-game cursor (sampled and drawn within the same frame as
    // gameplay) stays glued to whatever else the game draws against it
    // (aim lines, crosshairs). Restores the OS cursor at destruction.
    class Cursor {
        App& m_app;
        TextureId m_texture_id = INVALID_TEXTURE_ID;
        Vector2 m_pivot = Vector2(0.5f, 0.5f);
        Vector2 m_scale = Vector2(1.0f, 1.0f);
        Color m_tint = Color::white();
        bool m_visible = true;

    public:
        explicit Cursor(App& app);
        ~Cursor();

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

        bool is_visible() const;
        void set_visible(bool visible);

        void render();
    };
}
