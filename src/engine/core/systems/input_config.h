#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace hob {
    enum class InputDevice {
        Keyboard,
        Mouse,
        Gamepad,
    };

    // `code` values for InputSource when device == Mouse. SDL has no string lookup
    // for the mouse, so these are resolved by a small table in input_config.cpp and
    // interpreted by Input.
    enum class MouseCode {
        ButtonLeft,
        ButtonRight,
        ButtonMiddle,
        WheelUp,   // digital: fires for one frame on wheel up
        WheelDown, // digital: fires for one frame on wheel down
        AxisX,     // analog: motion delta x
        AxisY,     // analog: motion delta y
        AxisWheel, // analog: wheel delta y
    };

    // A single physical input. `code` is interpreted per device:
    //   Keyboard -> SDL_Scancode
    //   Mouse    -> MouseCode
    //   Gamepad  -> SDL_GamepadButton (digital) or SDL_GamepadAxis (analog)
    struct InputSource {
        InputDevice device = InputDevice::Keyboard;
        int code = 0;
        bool is_analog = false;
    };

    // An analog source feeding an axis. `scale` handles inversion (-1) and
    // sensitivity (e.g. 0.05 for mouse motion); it is read from config, never
    // encoded in the source name.
    struct AnalogBinding {
        InputSource source;
        float scale = 1.0f;
    };

    struct ActionConfig {
        std::vector<InputSource> sources;
    };

    struct AxisConfig {
        float acceleration = 0.0f;
        float deceleration = 0.0f;
        std::vector<InputSource> positive_sources;
        std::vector<InputSource> negative_sources;
        std::vector<AnalogBinding> analog;
    };

    struct GamepadTuning {
        float stick_deadzone = 0.2f;
        float trigger_deadzone = 0.1f;
        // Deadzoned analog magnitude at which a trigger (or other analog source) bound to a
        // digital action counts as pressed.
        float trigger_button_threshold = 0.5f;
    };

    struct InputConfig {
        std::unordered_map<std::string, ActionConfig> actions;
        std::unordered_map<std::string, AxisConfig> axes;
        GamepadTuning gamepad;

        InputConfig() = default;
        explicit InputConfig(const std::filesystem::path& json_path);

        // Digital sources referenced anywhere in the config, deduplicated. Used by
        // Input to know which sources to sample for edge (press/release) detection.
        std::vector<InputSource> relevant_sources() const;
    };
} // namespace hob
