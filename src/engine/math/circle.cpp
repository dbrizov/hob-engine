#include "circle.h"

#include <format>

namespace hob {
    std::string Circle::to_string() const {
        return std::format("Circle(center={}, radius={:.2f})", center.to_string(), radius);
    }
}
