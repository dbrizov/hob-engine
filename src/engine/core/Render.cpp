#include "Render.h"

#include <fmt/format.h>

Color::Color()
    : Color(0, 0, 0, 0) {
}

Color::Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
    : r(r_), g(g_), b(b_), a(a_) {
}

Color Color::black() {
    return Color(0, 0, 0);
}

Color Color::white() {
    return Color(255, 255, 255);
}

Color Color::gray() {
    return Color(128, 128, 128);
}

Color Color::red() {
    return Color(255, 0, 0);
}

Color Color::green() {
    return Color(0, 255, 0);
}

Color Color::blue() {
    return Color(0, 0, 255);
}

Color Color::yellow() {
    return Color(255, 255, 0);
}

Color Color::magenta() {
    return Color(255, 0, 255);
}

Color Color::cyan() {
    return Color(0, 255, 255);
}

Color Color::orange() {
    return Color(255, 165, 0);
}

std::string Color::to_string() const {
    return fmt::format("({}, {}, {}, {})", r, g, b, a);
}
