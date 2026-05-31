#pragma once

#include <string>

namespace hob {
    struct Color {
        float r;
        float g;
        float b;
        float a;

        Color();
        Color(float r_, float g_, float b_, float a_ = 1.0f);

        std::string to_string() const;

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
    };
}
