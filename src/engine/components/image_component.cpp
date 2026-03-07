#include "image_component.h"

#include "transform_component.h"
#include "engine/core/render.h"

namespace hob {
    ImageComponent::ImageComponent(Entity& entity)
        : Component(entity) {
    }

    TextureId ImageComponent::get_texture_id() const {
        return m_texture_id;
    }

    void ImageComponent::set_texture_id(TextureId texture_id) {
        m_texture_id = texture_id;
    }

    Vector2 ImageComponent::get_pivot() const {
        return m_pivot;
    }

    void ImageComponent::set_pivot(const Vector2& pivot) {
        m_pivot = pivot;
    }

    Vector2 ImageComponent::get_scale() const {
        return m_scale;
    }

    void ImageComponent::set_scale(const Vector2& scale) {
        m_scale = scale;
    }

    int ImageComponent::get_z_index() const {
        return m_z_index;
    }

    void ImageComponent::set_z_index(int z_index) {
        m_z_index = z_index;
    }
}
