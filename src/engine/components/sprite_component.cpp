#include "sprite_component.h"

#include <format>

#include "engine/entity/entity.h"
#include "transform_component.h"

namespace hob {
    SpriteComponent::SpriteComponent(Entity& entity)
        : Component(entity) {
    }

    std::string SpriteComponent::to_string() const {
        return std::format("SpriteComponent(entity_id = {})", get_entity().get_id());
    }

    TextureId SpriteComponent::get_texture_id() const {
        return m_texture_id;
    }

    void SpriteComponent::set_texture_id(TextureId texture_id) {
        m_texture_id = texture_id;
    }

    Vector2 SpriteComponent::get_pivot() const {
        return m_pivot;
    }

    void SpriteComponent::set_pivot(const Vector2& pivot) {
        m_pivot = pivot;
    }

    Vector2 SpriteComponent::get_scale() const {
        return m_scale;
    }

    void SpriteComponent::set_scale(const Vector2& scale) {
        m_scale = scale;
    }

    Color SpriteComponent::get_tint() const {
        return m_tint;
    }

    void SpriteComponent::set_tint(const Color& color) {
        m_tint = color;
    }

    int SpriteComponent::get_z_index() const {
        return m_z_index;
    }

    void SpriteComponent::set_z_index(int z_index) {
        m_z_index = z_index;
    }
}
