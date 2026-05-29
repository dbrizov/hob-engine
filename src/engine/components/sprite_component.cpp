#include "sprite_component.h"

#include <format>

#include "engine/core/app.h"
#include "engine/core/path_utils.h"
#include "engine/entity/entity.h"
#include "transform_component.h"

namespace hob {
    SpriteComponent::SpriteComponent(Entity& entity)
        : Component(entity) {
    }

    SpriteComponent::~SpriteComponent() {
        clear_texture();
    }

    std::string SpriteComponent::to_string() const {
        return std::format("SpriteComponent(entity_id = {})", get_entity().get_id());
    }

    TextureId SpriteComponent::get_texture_id() const {
        return m_texture_id;
    }

    void SpriteComponent::set_texture(const std::string& relative_path) {
        Assets& assets = get_app().get_assets();
        const std::filesystem::path full_path = PathUtils::get_assets_root_path() / relative_path;
        const TextureId new_id = assets.load_texture(full_path);

        if (m_texture_id != INVALID_TEXTURE_ID) {
            assets.unload_texture(m_texture_id);
        }

        m_texture_id = new_id;
    }

    void SpriteComponent::clear_texture() {
        if (m_texture_id != INVALID_TEXTURE_ID) {
            get_app().get_assets().unload_texture(m_texture_id);
            m_texture_id = INVALID_TEXTURE_ID;
        }
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
