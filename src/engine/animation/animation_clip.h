#pragma once

#include <vector>

#include "engine/core/systems/renderer/texture.h"

namespace hob {
    struct AnimationFrame {
        TextureRef texture;
    };

    struct AnimationClip {
        std::vector<AnimationFrame> frames;
        float fps = 12.0f;
        bool looping = true;
    };
}
