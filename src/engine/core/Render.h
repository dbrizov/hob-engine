#ifndef HOB_ENGINE_RENDER_H
#define HOB_ENGINE_RENDER_H
#include <span>
#include <vector>

#include "Assets.h"
#include "engine/math/Vector2.h"


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


#endif //HOB_ENGINE_RENDER_H
