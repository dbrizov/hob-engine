#include "vector2.h"

#include <format>

#include "matrix2x3.h"

namespace hob {
    std::string Vector2::to_string() const {
        return std::format("({:.2f}, {:.2f})", x, y);
    }

    Vector2 Vector2::rotate_around(const Vector2& point, const Vector2& pivot, float radians) {
        const Matrix2x3 rotation_matrix = Matrix2x3::make_rotate_around(pivot, radians);
        const Vector2 rotated_point = rotation_matrix.transform_point(point);

        return rotated_point;
    }
} // namespace hob
