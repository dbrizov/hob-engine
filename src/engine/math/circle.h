#pragma once

#include <cmath>
#include <string>

#include "constants.h"
#include "vector2.h"

namespace hob {
    struct Circle {
        Vector2 center;
        float radius;

        constexpr Circle(const Vector2& center_, float radius_)
            : center(center_)
            , radius(radius_) {}

        std::string to_string() const;

        bool operator==(const Circle& right) const {
            return center == right.center && std::abs(radius - right.radius) < EPSILON;
        }

        bool operator!=(const Circle& right) const { return !operator==(right); }
    };
}
