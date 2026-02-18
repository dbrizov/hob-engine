#include "ImageComponent.h"

#include "TransformComponent.h"
#include "engine/core/Render.h"

ImageComponent::ImageComponent(Entity& entity)
    : Component(entity) {
}

int ImageComponent::get_priority() const {
    return component_priority::CP_RENDER;
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
