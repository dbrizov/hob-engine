#include "color.h"

#include <format>

namespace hob {
    std::string Color::to_string() const {
        return std::format("({}, {}, {}, {})", r, g, b, a);
    }
}
