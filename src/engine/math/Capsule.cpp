#include "Capsule.h"

namespace hob {
    Capsule::Capsule(const Vector2& center_a_, const Vector2& center_b_, float radius_)
        : center_a(center_a_), center_b(center_b_), radius(radius_) {
    }

    float Capsule::get_height() const {
        float height = Vector2::distance(center_a, center_b) + (radius * 2.0f);
        return height;
    }
}
