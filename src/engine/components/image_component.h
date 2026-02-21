#ifndef HOB_ENGINE_IMAGECOMPONENT_H
#define HOB_ENGINE_IMAGECOMPONENT_H
#include "component.h"
#include "engine/core/assets.h"
#include "engine/math/vector2.h"


namespace hob {
    class ImageComponent : public Component {
        TextureId m_texture_id = INVALID_TEXTURE_ID;
        Vector2 m_pivot = Vector2(0.5f, 0.5f);
        Vector2 m_scale = Vector2(1.0f, 1.0f);

    public:
        explicit ImageComponent(Entity& entity);

        virtual int get_priority() const override;

        TextureId get_texture_id() const;
        void set_texture_id(TextureId texture_id);

        Vector2 get_pivot() const;
        void set_pivot(const Vector2& pivot);

        Vector2 get_scale() const;
        void set_scale(const Vector2& scale);
    };
}


#endif //HOB_ENGINE_IMAGECOMPONENT_H
