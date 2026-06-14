#pragma once

#include <SDL3/SDL_scancode.h>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace hob {
    struct ActionConfig {
        std::vector<SDL_Scancode> keys;
    };

    struct AxisConfig {
        float acceleration;
        float deceleration;
        std::vector<SDL_Scancode> positive_keys;
        std::vector<SDL_Scancode> negative_keys;
    };

    struct InputConfig {
        std::unordered_map<std::string, ActionConfig> actions;
        std::unordered_map<std::string, AxisConfig> axes;

        InputConfig() = default;
        explicit InputConfig(const std::filesystem::path& json_path);

        std::vector<SDL_Scancode> relevant_keys() const;
    };
} // namespace hob
