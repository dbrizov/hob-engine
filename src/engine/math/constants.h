#pragma once

#include <cfloat>

namespace hob {
    constexpr float EPSILON = 0.0001f;
    constexpr float PI = 3.1415926535f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;

    constexpr int32_t MIN_INT32 = INT32_MIN;
    constexpr int32_t MAX_INT32 = INT32_MAX;
    constexpr uint32_t MAX_UINT32 = UINT32_MAX;
    constexpr int64_t MIN_INT64 = INT64_MIN;
    constexpr int64_t MAX_INT64 = INT64_MAX;
    constexpr uint64_t MAX_UINT64 = UINT64_MAX;
    constexpr float MIN_FLOAT = -FLT_MAX;
    constexpr float MAX_FLOAT = FLT_MAX;
    constexpr double MIN_DOUBLE = -DBL_MAX;
    constexpr double MAX_DOUBLE = DBL_MAX;
}
