#pragma once

#include <cstdint>
#include <string>

#include "component.h"
#include "engine/core/systems/renderer.h"
#include "engine/math/vector2.h"

namespace hob {
    class SpriteComponent : public Component {
        TextureRef m_texture;
        Material m_material;
        Vector2 m_pivot = Vector2(0.5f, 0.5f);
        Vector2 m_scale = Vector2(1.0f, 1.0f);
        int m_z_index = 0;
        uint32_t m_pixels_per_meter = 64;

    public:
        explicit SpriteComponent(Entity& entity);

        std::string to_string() const override;

        bool has_texture() const;
        const TextureRef& get_texture() const;
        void set_texture(const std::string& path);
        void clear_texture();

        const Material& get_material() const;
        Material& get_material();
        void set_material(const Material& material);

        Vector2 get_pivot() const;
        void set_pivot(const Vector2& pivot);

        Vector2 get_scale() const;
        void set_scale(const Vector2& scale);

        int get_z_index() const;
        void set_z_index(int z_index);

        uint32_t get_pixels_per_meter() const;
        float get_pixels_per_meter_f() const;
        void set_pixels_per_meter(uint32_t value);
    };
}
