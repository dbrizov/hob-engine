#include "mathf.h"

#include <cmath>

namespace hob::math {
    float normalize_angle(float angle_deg) {
        angle_deg = std::fmod(angle_deg, 360.0f);
        if (angle_deg < 0) {
            angle_deg += 360.0f;
        }

        return angle_deg;
    }

    float lerp(float a, float b, float t) {
        return a * (1.0f - t) + b * t;
    }

    float lerp_angle(float a_deg, float b_deg, float t) {
        const float diff = std::remainder(b_deg - a_deg, 360.0f); // in range [-180, 180]
        const float b_wrapped = a_deg + diff;
        const float result = a_deg * (1.0f - t) + b_wrapped * t;
        const float normalized = normalize_angle(result);

        return normalized;
    }
} // hob
