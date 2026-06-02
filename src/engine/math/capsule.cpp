#include "capsule.h"

#include <format>

namespace hob {
    std::string Capsule::to_string() const {
        return std::format("Capsule(center_a={}, center_b={}, radius={:.2f})",
                           center_a.to_string(),
                           center_b.to_string(),
                           radius);
    }
}
