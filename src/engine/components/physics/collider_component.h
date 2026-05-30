#pragma once

#include <box2d/id.h>
#include <box2d/types.h>

#include "engine/components/component.h"
#include "engine/math/vector2.h"

namespace hob {
    struct Color;

    class ColliderComponent : public Component {
        b2ShapeId m_shape_id = b2_nullShapeId;
        float m_density = 1.0f; // In kg/m^2. A body's mass = density * area
        float m_friction = 0.6f; // In range [0, 1]
        float m_bounciness = 0.0f; // In range [0, 1]
        uint64_t m_collision_layer = 1u; // The collision layer of this collider
        uint64_t m_collision_mask = ~0ull; // What this collider collides with
        bool m_is_trigger = false;

        // Transform scale baked into the physics shape; refreshed on enter_play / on_scale_changed.
        Vector2 m_baked_scale = Vector2(1.0f, 1.0f);

    public:
        explicit ColliderComponent(Entity& entity);

        virtual void enter_play() override;
        virtual void exit_play() override;
        virtual void debug_draw_tick(float delta_time) override;

        std::string to_string() const override;

        b2BodyId get_body_id() const;
        b2ShapeId get_shape_id() const;

        float get_density() const;
        void set_density(float density);

        float get_friction() const;
        void set_friction(float friction);

        float get_bounciness() const;
        void set_bounciness(float bounciness);

        uint64_t get_collision_layer() const;
        void set_collision_layer(uint64_t collision_layer);

        uint64_t get_collision_mask() const;
        void set_collision_mask(uint64_t collision_mask);

        bool is_trigger() const;
        void set_trigger(bool trigger);

        Vector2 get_baked_scale() const;

        // Rebuilds the physics shape if the transform's scale changed.
        // Auto-invoked by TransformComponent::set_scale.
        void on_scale_changed();

    protected:
        virtual b2ShapeId create_shape(const b2ShapeDef& shape_def, const Vector2& scale) = 0;
        virtual void rebuild_shape(const Vector2& scale) = 0;
        virtual void debug_draw_shape(const Color& color, const Vector2& scale) const = 0;
    };
}
