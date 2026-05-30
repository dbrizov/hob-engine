#include "sprite_component.h"

#include <format>

#include "engine/core/engine.h"
#include "engine/core/path_utils.h"
#include "engine/entity/entity.h"
#include "transform_component.h"

namespace hob {
    SpriteComponent::SpriteComponent(Entity& entity)
        : Component(entity) {
    }

    void SpriteComponent::init() {
        m_pixels_per_meter = get_engine().get_renderer().get_pixels_per_meter();
    }

    std::string SpriteComponent::to_string() const {
        return std::format("SpriteComponent(entity_id = {})", get_entity().get_id());
    }

    bool SpriteComponent::has_texture() const {
        return m_texture.is_valid();
    }

    const TextureRef& SpriteComponent::get_texture() const {
        return m_texture;
    }

    void SpriteComponent::set_texture(const std::string& relative_path) {
        const std::filesystem::path full_path = PathUtils::get_assets_root_path() / relative_path;
        m_texture = get_engine().get_renderer().load_texture(full_path);
    }

    void SpriteComponent::clear_texture() {
        m_texture.reset();
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

    uint32_t SpriteComponent::get_pixels_per_meter() const {
        return m_pixels_per_meter;
    }

    float SpriteComponent::get_pixels_per_meter_f() const {
        return static_cast<float>(m_pixels_per_meter);
    }

    void SpriteComponent::set_pixels_per_meter(uint32_t value) {
        m_pixels_per_meter = value;
    }
}
