#include "matrix2x3.h"

#include <cmath>

#include "constants.h"

namespace hob {
    float Matrix2x3::get_rotation_degrees() const {
        float rotation_rad = std::atan2(x.y, x.x); // rotation from basis X direction
        float rotation_deg = rotation_rad * RAD_TO_DEG;

        return rotation_deg;
    }

    Vector2 Matrix2x3::get_scale() const {
        return Vector2(x.length(), y.length());
    }

    Vector2 Matrix2x3::transform_point(Vector2 p) const {
        return origin + x * p.x + y * p.y;
    }

    Matrix2x3 Matrix2x3::multiply(const Matrix2x3& a, const Matrix2x3& b) {
        Matrix2x3 out;
        out.x = a.x * b.x.x + a.y * b.x.y;
        out.y = a.x * b.y.x + a.y * b.y.y;
        out.origin = a.origin + a.x * b.origin.x + a.y * b.origin.y;

        return out;
    }

    Matrix2x3 Matrix2x3::lerp(const Matrix2x3& a, const Matrix2x3& b, float t) {
        return {
            a.x * (1.0f - t) + b.x * t,
            a.y * (1.0f - t) + b.y * t,
            a.origin * (1.0f - t) + b.origin * t
        };
    }

    Matrix2x3 Matrix2x3::make_rotate_around(const Vector2& center, float degrees) {
        float rad = degrees * DEG_TO_RAD;
        float cos = std::cos(rad);
        float sin = std::sin(rad);

        Matrix2x3 matrix;
        matrix.x = Vector2(cos, sin);
        matrix.y = Vector2(-sin, cos);
        matrix.origin = center - (matrix.x * center.x + matrix.y * center.y);

        return matrix;
    }
}
