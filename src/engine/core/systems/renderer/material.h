#pragma once

#include <cstdint>

#include "engine/math/color.h"

namespace hob {
    using ShaderId = int32_t;
    constexpr ShaderId INVALID_SHADER_ID = -1;
    constexpr ShaderId DEFAULT_SPRITE_SHADER_ID = 0;

    struct Material {
        ShaderId shader_id = DEFAULT_SPRITE_SHADER_ID;
        Color tint = Color::white();
        Color outline_color = Color::white();
        float outline_width = 0.0f;
        float alpha_threshold = 0.1f;
    };
}
