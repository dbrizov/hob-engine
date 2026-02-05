#ifndef CPP_PLATFORMER_RENDER_H
#define CPP_PLATFORMER_RENDER_H
#include <span>
#include <vector>

#include "Assets.h"
#include "engine/math/Vector2.h"


struct RenderData {
    TextureId texture_id = INVALID_TEXTURE_ID;
    Vector2 position;
    Vector2 prev_position;
    Vector2 scale;

    RenderData(TextureId texture_id_, Vector2 position_, Vector2 prev_position_, Vector2 scale_)
        : texture_id(texture_id_)
          , position(position_)
          , prev_position(prev_position_)
          , scale(scale_) {
    }
};


class RenderQueue {
    std::vector<RenderData> m_render_data;

public:
    void enqueue(const RenderData& data);
    void clear();

    std::span<const RenderData> get_render_data() const;
};


#endif //CPP_PLATFORMER_RENDER_H
