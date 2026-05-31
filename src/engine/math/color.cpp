#include "color.h"

#include <format>

namespace hob {
    Color::Color()
        : Color(0.0f, 0.0f, 0.0f, 0.0f) {
    }

    Color::Color(float r_, float g_, float b_, float a_)
        : r(r_)
        , g(g_)
        , b(b_)
        , a(a_) {
    }

    std::string Color::to_string() const {
        return std::format("({}, {}, {}, {})", r, g, b, a);
    }

    Color Color::black() { return Color(0.0f, 0.0f, 0.0f); }
    Color Color::white() { return Color(1.0f, 1.0f, 1.0f); }
    Color Color::gray() { return Color(0.5f, 0.5f, 0.5f); }
    Color Color::red() { return Color(1.0f, 0.0f, 0.0f); }
    Color Color::green() { return Color(0.0f, 1.0f, 0.0f); }
    Color Color::blue() { return Color(0.0f, 0.0f, 1.0f); }
    Color Color::yellow() { return Color(1.0f, 1.0f, 0.0f); }
    Color Color::magenta() { return Color(1.0f, 0.0f, 1.0f); }
    Color Color::cyan() { return Color(0.0f, 1.0f, 1.0f); }
    Color Color::orange() { return Color(1.0f, 0.647f, 0.0f); }
}
