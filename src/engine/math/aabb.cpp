#include "aabb.h"

namespace hob {
    AABB::AABB(const Vector2& center_, const Vector2& extents_)
        : center(center_), extents(extents_) {
    }

    Vector2 AABB::min() const {
        return center - extents;
    }

    Vector2 AABB::max() const {
        return center + extents;
    }

    Vector2 AABB::size() const {
        return extents * 2.0f;
    }
}
