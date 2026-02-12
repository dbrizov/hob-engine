#include "ImageComponent.h"

#include "TransformComponent.h"
#include "engine/core/Render.h"
#include "engine/entity/Entity.h"

ImageComponent::ImageComponent(Entity& entity)
    : Component(entity) {
}

ComponentPriority ImageComponent::get_priority() const {
    return ComponentPriority::RENDER;
}

void ImageComponent::render_tick(float delta_time, RenderQueue& render_queue) {
    const TransformComponent* transform = get_entity().get_transform();
    Vector2 position = transform->get_position();
    Vector2 prev_position = transform->get_prev_position();
    Vector2 t_scale = transform->get_scale();
    Vector2 scale = Vector2(m_scale.x * t_scale.x, m_scale.y * t_scale.y);

    render_queue.enqueue(RenderData(
        m_texture_id,
        position,
        prev_position,
        scale));
}

TextureId ImageComponent::get_texture_id() const {
    return m_texture_id;
}

void ImageComponent::set_texture_id(TextureId texture_id) {
    m_texture_id = texture_id;
}

Vector2 ImageComponent::get_scale() const {
    return m_scale;
}

void ImageComponent::set_scale(Vector2 scale) {
    m_scale = scale;
}
