#include "mathf.h"

#include <cmath>

namespace hob::math {
    float normalize_angle(float angle) {
        angle = std::fmod(angle, 360.0f);
        if (angle < 0) {
            angle += 360.0f;
        }

        return angle;
    }

    float lerp(float a, float b, float t) {
        return a * (1.0f - t) + b * t;
    }

    float lerp_angle(float a, float b, float t) {
        float diff = std::remainder(b - a, 360.0f); // in range [-180, 180]
        float b_wrapped = a + diff;

        return a * (1.0f - t) + b_wrapped * t;
    }
} // hob
