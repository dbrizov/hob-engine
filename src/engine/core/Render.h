#ifndef CPP_PLATFORMER_RENDER_H
#define CPP_PLATFORMER_RENDER_H
#include <span>
#include <vector>

#include "Assets.h"
#include "engine/math/Vector2.h"


struct RenderData {
    TextureId texture_id = 0;
    Vector2 position;
    Vector2 prev_position;
    Vector2 scale;
};


class RenderQueue {
    std::vector<RenderData> m_render_data;

public:
    void enqueue(RenderData data);
    void clear();

    std::span<const RenderData> get_render_data();
};


#endif //CPP_PLATFORMER_RENDER_H
