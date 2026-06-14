#pragma once

#include <limits>
#include <numbers>

namespace hob {
    constexpr float EPSILON = 0.0001f;
    constexpr float PI = std::numbers::pi_v<float>;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;

    constexpr int32_t MIN_INT32 = std::numeric_limits<int32_t>::min();
    constexpr int32_t MAX_INT32 = std::numeric_limits<int32_t>::max();
    constexpr uint32_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
    constexpr int64_t MIN_INT64 = std::numeric_limits<int64_t>::min();
    constexpr int64_t MAX_INT64 = std::numeric_limits<int64_t>::max();
    constexpr uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();
    constexpr float MIN_FLOAT = std::numeric_limits<float>::lowest();
    constexpr float MAX_FLOAT = std::numeric_limits<float>::max();
    constexpr double MIN_DOUBLE = std::numeric_limits<double>::lowest();
    constexpr double MAX_DOUBLE = std::numeric_limits<double>::max();
} // namespace hob
