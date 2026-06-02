#include "aabb.h"

#include <format>

namespace hob {
    std::string AABB::to_string() const {
        return std::format("AABB(center={}, extents={})", center.to_string(), extents.to_string());
    }
}
