#pragma once

#include "vector2.h"

namespace hob {
    struct Matrix2x3 {
        Vector2 x; // basis X axis (rotation + scale X)
        Vector2 y; // basis Y axis (rotation + scale Y)
        Vector2 origin; // translation (position)

        float get_rotation() const {
            return std::atan2(x.y, x.x);
        }

        Vector2 get_scale() const {
            return Vector2(x.length(), y.length());
        }

        Vector2 transform_point(const Vector2& p) const {
            return origin + x * p.x + y * p.y;
        }

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
            return {
                Vector2::right(),
                Vector2::up(),
                Vector2::zero()
            };
        }

        static Matrix2x3 multiply(const Matrix2x3& a, const Matrix2x3& b) {
            Matrix2x3 out;
            out.x = a.x * b.x.x + a.y * b.x.y;
            out.y = a.x * b.y.x + a.y * b.y.y;
            out.origin = a.origin + a.x * b.origin.x + a.y * b.origin.y;
            return out;
        }

        static Matrix2x3 lerp(const Matrix2x3& a, const Matrix2x3& b, float t) {
            return {
                a.x * (1.0f - t) + b.x * t,
                a.y * (1.0f - t) + b.y * t,
                a.origin * (1.0f - t) + b.origin * t
            };
        }

        static Matrix2x3 make_rotate_around(const Vector2& pivot, float radians);
    };
}
