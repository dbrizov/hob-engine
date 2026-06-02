#pragma once

#include <string>

#include "vector2.h"

namespace hob {
    struct AABB {
        Vector2 center;
        Vector2 extents;

        constexpr AABB(const Vector2& center_, const Vector2& extents_)
            : center(center_)
            , extents(extents_) {
        }

        std::string to_string() const;

        Vector2 min() const { return center - extents; }
        Vector2 max() const { return center + extents; }
        Vector2 size() const { return extents * 2.0f; }
    };
}
