#ifndef CPP_PLATFORMER_RENDER_H
#define CPP_PLATFORMER_RENDER_H
#include <span>
#include <vector>

#include "Assets.h"
#include "engine/math/Vector2.h"


#pragma region Color
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color();
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255);

    static Color black();
    static Color white();
    static Color gray();
    static Color red();
    static Color green();
    static Color blue();
    static Color yellow();
    static Color magenta();
    static Color cyan();
    static Color orange();

    std::string to_string() const;
};
#pragma endregion Color


#pragma region RenderQueue
struct RenderData {
    TextureId texture_id;
    Vector2 position;
    Vector2 prev_position;
    Vector2 scale;

    RenderData(TextureId texture_id_, const Vector2& position_, const Vector2& prev_position_, const Vector2& scale_);
};


class RenderQueue {
    std::vector<RenderData> m_render_data;

public:
    void enqueue(const RenderData& data);
    void clear();

    std::span<const RenderData> get_render_data() const;
};
#pragma endregion RenderQueue


#endif //CPP_PLATFORMER_RENDER_H
