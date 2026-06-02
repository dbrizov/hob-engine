#pragma once

#include <string>

#include "vector2.h"

namespace hob {
    struct Capsule {
        Vector2 center_a;
        Vector2 center_b;
        float radius;

        constexpr Capsule(const Vector2& center_a_, const Vector2& center_b_, float radius_)
            : center_a(center_a_)
            , center_b(center_b_)
            , radius(radius_) {
        }

        std::string to_string() const;

        float get_height() const {
            return Vector2::distance(center_a, center_b) + (radius * 2.0f);
        }
    };
}
