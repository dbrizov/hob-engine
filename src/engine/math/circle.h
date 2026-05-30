#pragma once

#include "vector2.h"

namespace hob {
    struct Circle {
        Vector2 center;
        float radius;

        Circle(const Vector2& center_, float radius_);
    };
}
