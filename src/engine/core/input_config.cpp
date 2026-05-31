#include "input_config.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

#include "logging.h"

namespace hob {
    static SDL_Scancode scancode_from_name(const std::string& name) {
        SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
        if (scancode == SDL_SCANCODE_UNKNOWN) {
            debug::log_error("Unknown key: {}", name);
        }

        return scancode;
    }

    InputConfig::InputConfig(const std::filesystem::path& json_path) {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            debug::log_error("Cannot open input config file: {}", json_path.string());
            return;
        }

        nlohmann::json json = nlohmann::json::parse(file);

        // Actions
        for (auto& [action_name, keys] : json["action_mappings"].items()) {
            ActionConfig action_config;
            for (auto& key : keys) {
                SDL_Scancode scancode = scancode_from_name(key.get<std::string>());
                if (scancode != SDL_SCANCODE_UNKNOWN) {
                    action_config.keys.push_back(scancode);
                }
            }

            actions[action_name] = action_config;
        }

        // Axes
        for (auto& [axis_name, cfg] : json["axis_mappings"].items()) {
            AxisConfig axis_config;
            axis_config.acceleration = cfg["acceleration"].get<float>();
            axis_config.deceleration = cfg["deceleration"].get<float>();

            for (auto& key : cfg["positive"]) {
                SDL_Scancode scancode = scancode_from_name(key.get<std::string>());
                if (scancode != SDL_SCANCODE_UNKNOWN) {
                    axis_config.positive_keys.push_back(scancode);
                }
            }

            for (auto& key : cfg["negative"]) {
                SDL_Scancode scancode = scancode_from_name(key.get<std::string>());
                if (scancode != SDL_SCANCODE_UNKNOWN) {
                    axis_config.negative_keys.push_back(scancode);
                }
            }

            axes[axis_name] = axis_config;
        }
    }

    std::vector<SDL_Scancode> InputConfig::relevant_keys() const {
        std::vector<SDL_Scancode> keys;
        keys.reserve(32);

        auto add_key = [&keys](SDL_Scancode key) {
            if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
                keys.push_back(key);
            }
        };

        for (const auto& pair : actions) {
            for (auto key : pair.second.keys) {
                add_key(key);
            }
        }

        for (const auto& pair : axes) {
            for (auto key : pair.second.positive_keys) {
                add_key(key);
            }

            for (auto key : pair.second.negative_keys) {
                add_key(key);
            }
        }

        return keys;
    }
}
