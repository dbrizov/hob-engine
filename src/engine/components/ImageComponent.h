#ifndef CPP_PLATFORMER_IMAGECOMPONENT_H
#define CPP_PLATFORMER_IMAGECOMPONENT_H
#include "Component.h"
#include "engine/core/Assets.h"
#include "engine/math/Vector2.h"


class ImageComponent : public Component {
    TextureId m_texture_id = INVALID_TEXTURE_ID;
    Vector2 m_scale = Vector2(1.0f, 1.0f);

public:
    virtual ComponentPriority get_priority() const override;

    virtual void render_tick(float delta_time, RenderQueue& render_queue) override;

    TextureId get_texture_id() const;
    void set_texture_id(TextureId texture_id);

    Vector2 get_scale() const;
    void set_scale(Vector2 scale);
};


#endif //CPP_PLATFORMER_IMAGECOMPONENT_H
