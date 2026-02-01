#include "Render.h"


void RenderQueue::enqueue(RenderData data) {
    m_render_data.push_back(data);
}

void RenderQueue::clear() {
    m_render_data.clear();
}

std::span<RenderData const> RenderQueue::get_render_data() {
    return m_render_data;
}
