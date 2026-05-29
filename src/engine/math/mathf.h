#pragma once

namespace hob::math {
    float normalize_angle(float angle_deg);
    float lerp(float a, float b, float t);
    float lerp_angle(float a_deg, float b_deg, float t);
}
