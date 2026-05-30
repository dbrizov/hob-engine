#pragma once

#include <string>

#include "component.h"
#include "engine/core/renderer.h"
#include "engine/math/vector2.h"

namespace hob {
    class SpriteComponent : public Component {
        TextureRef m_texture;
        Vector2 m_pivot = Vector2(0.5f, 0.5f);
        Vector2 m_scale = Vector2(1.0f, 1.0f);
        Color m_tint = Color::white();
        int m_z_index = 0;

    public:
        explicit SpriteComponent(Entity& entity);

        std::string to_string() const override;

        bool has_texture() const;
        const TextureRef& get_texture() const;
        void set_texture(const std::string& relative_path);
        void clear_texture();

        Vector2 get_pivot() const;
        void set_pivot(const Vector2& pivot);

        Vector2 get_scale() const;
        void set_scale(const Vector2& scale);

        Color get_tint() const;
        void set_tint(const Color& color);

        int get_z_index() const;
        void set_z_index(int z_index);
    };
}
