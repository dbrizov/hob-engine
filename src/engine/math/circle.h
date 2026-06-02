#pragma once

#include <string>

#include "vector2.h"

namespace hob {
    struct Circle {
        Vector2 center;
        float radius;

        constexpr Circle(const Vector2& center_, float radius_)
            : center(center_)
            , radius(radius_) {
        }

        std::string to_string() const;
    };
}
