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

    Vector2 Matrix2x3::transform_point(const Matrix2x3& m, Vector2 p) {
        return m.origin + m.x * p.x + m.y * p.y;
    }
}
