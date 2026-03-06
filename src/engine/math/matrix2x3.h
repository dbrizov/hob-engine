#pragma once

#include "vector2.h"

namespace hob {
    struct Matrix2x3 {
        Vector2 x; // basis X axis (rotation + scale X)
        Vector2 y; // basis Y axis (rotation + scale Y)
        Vector2 origin; // translation (position)

        float get_rotation_degrees() const;
        Vector2 get_scale() const;

        static Matrix2x3 multiply(const Matrix2x3& a, const Matrix2x3& b);
        static Matrix2x3 lerp(const Matrix2x3& a, const Matrix2x3& b, float t);
        static Matrix2x3 make_rotate_around(const Vector2& center, float degrees);
        static Vector2 transform_point(const Matrix2x3& m, Vector2 p);
    };
}
