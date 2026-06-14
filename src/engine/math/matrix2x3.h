#pragma once

#include <cmath>

#include "vector2.h"

namespace hob {
    struct Matrix2x3 {
        Vector2 x; // basis X axis (rotation + scale X)
        Vector2 y; // basis Y axis (rotation + scale Y)
        Vector2 origin; // translation (position)

        float get_rotation() const { return std::atan2(x.y, x.x); }

        Vector2 get_scale() const {
            // A reflection (negative determinant) can't be split into unique per-axis signs, so by
            // convention attach the flip to Y. Combined with get_rotation() this still reconstructs the
            // matrix, which is what lets a mirrored parent flip its children.
            const float det = x.x * y.y - y.x * x.y;
            const float sign_y = (det < 0.0f) ? -1.0f : 1.0f;
            return Vector2(x.length(), sign_y * y.length());
        }

        Vector2 transform_point(const Vector2& p) const { return origin + x * p.x + y * p.y; }

        Matrix2x3 inverse() const {
            const float det = x.x * y.y - y.x * x.y;
            if (std::abs(det) <= EPSILON) {
                return Matrix2x3::identity();
            }

            const float inv_det = 1.0f / det;
            Matrix2x3 out;
            out.x = Vector2(y.y * inv_det, -x.y * inv_det);
            out.y = Vector2(-y.x * inv_det, x.x * inv_det);
            out.origin = -(out.x * origin.x + out.y * origin.y);
            return out;
        }

        static Matrix2x3 identity() {
            Matrix2x3 out;
            out.x = Vector2::right();
            out.y = Vector2::up();
            out.origin = Vector2::zero();
            return out;
        }

        static Matrix2x3 multiply(const Matrix2x3& a, const Matrix2x3& b) {
            Matrix2x3 out;
            out.x = a.x * b.x.x + a.y * b.x.y;
            out.y = a.x * b.y.x + a.y * b.y.y;
            out.origin = a.origin + a.x * b.origin.x + a.y * b.origin.y;
            return out;
        }

        static Matrix2x3 lerp(const Matrix2x3& a, const Matrix2x3& b, float t) {
            Matrix2x3 out;
            out.x = a.x * (1.0f - t) + b.x * t;
            out.y = a.y * (1.0f - t) + b.y * t;
            out.origin = a.origin * (1.0f - t) + b.origin * t;
            return out;
        }

        static Matrix2x3 make_rotate_around(const Vector2& pivot, float radians) {
            const float cos = std::cos(radians);
            const float sin = std::sin(radians);

            Matrix2x3 out;
            out.x = Vector2(cos, sin);
            out.y = Vector2(-sin, cos);
            out.origin = pivot - (out.x * pivot.x + out.y * pivot.y);
            return out;
        }
    };

    inline Matrix2x3 operator*(const Matrix2x3& a, const Matrix2x3& b) {
        return Matrix2x3::multiply(a, b);
    }
}
