#include "matrix2x3.h"

#include <cmath>

namespace hob {
    Matrix2x3 Matrix2x3::make_rotate_around(const Vector2& pivot, float radians) {
        const float cos = std::cos(radians);
        const float sin = std::sin(radians);

        Matrix2x3 matrix;
        matrix.x = Vector2(cos, sin);
        matrix.y = Vector2(-sin, cos);
        matrix.origin = pivot - (matrix.x * pivot.x + matrix.y * pivot.y);

        return matrix;
    }
}
