#include "Input.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <SDL_keyboard.h>
#include <fmt/base.h>

// ---------------- InputMappings ----------------
static SDL_Scancode scancode_from_name(const std::string& name) {
    SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        fmt::println(stderr, "Unknown key: {}", name);
    }

    return scancode;
}

static InputMappings load_input_mappings(const std::filesystem::path& path) {
    InputMappings input_mappings;

    std::ifstream file(path);
    if (!file.is_open()) {
        fmt::println(stderr, "Cannot open input config file: {}", path.string());
        return input_mappings;
    }

    nlohmann::json json = nlohmann::json::parse(file);

    // Actions
    for (auto& [action, keys] : json["action_mappings"].items()) {
        std::vector<SDL_Scancode> scancodes;
        for (auto& key : keys) {
            SDL_Scancode scancode = scancode_from_name(key.get<std::string>());
            if (scancode != SDL_SCANCODE_UNKNOWN) {
                scancodes.push_back(scancode);
            }
        }

        input_mappings.actions[action] = scancodes;
    }

    // Axes
    for (auto& [axis, cfg] : json["axis_mappings"].items()) {
        AxisMapping axis_mappings;
        axis_mappings.acceleration = cfg["acceleration"].get<float>();
        axis_mappings.deceleration = cfg["deceleration"].get<float>();

        for (auto& key : cfg["positive"]) {
            SDL_Scancode scancode = scancode_from_name(key.get<std::string>());
            if (scancode != SDL_SCANCODE_UNKNOWN) {
                axis_mappings.positive.push_back(scancode);
            }
        }

        for (auto& key : cfg["negative"]) {
            SDL_Scancode scancode = scancode_from_name(key.get<std::string>());
            if (scancode != SDL_SCANCODE_UNKNOWN) {
                axis_mappings.negative.push_back(scancode);
            }
        }

        input_mappings.axes[axis] = axis_mappings;
    }

    return input_mappings;
}

InputEvent::InputEvent(const char* ev_name, InputEventType ev_type, float ev_axis_value)
    : name(ev_name)
    , type(ev_type)
    , axis_value(ev_axis_value) {}

std::vector<SDL_Scancode> InputMappings::relevant_keys() const {
    std::vector<SDL_Scancode> keys;
    keys.reserve(32);

    auto add_key = [&keys](SDL_Scancode key) {
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
            keys.push_back(key);
        }
    };

    for (const auto& pair : actions) {
        for (auto key : pair.second) {
            add_key(key);
        }
    }

    for (const auto& pair : axes) {
        for (auto key : pair.second.positive) {
            add_key(key);
        }

        for (auto key : pair.second.negative) {
            add_key(key);
        }
    }

    return keys;
}

// ---------------- Input ----------------
Input::Input(const std::filesystem::path& input_config_path) {
    m_input_mappings = load_input_mappings(input_config_path);
    m_relevant_keys = m_input_mappings.relevant_keys();

    for (const auto& [axis, _] : m_input_mappings.axes) {
        m_axis_values[axis] = 0.0f;
    }
}

void Input::tick(float delta_time, const Uint8* keyboard_state) {
    // TODO Optimize - have a map of pressed keys similar to SDL's keyboard_state
    update_pressed_keys(keyboard_state);

    // Dispatch action events
    for (auto& [action, keys] : m_input_mappings.actions) {
        for (auto key : keys) {
            bool pressed_now = std::find(m_pressed_keys_this_frame.begin(), m_pressed_keys_this_frame.end(), key) != m_pressed_keys_this_frame.end();
            bool pressed_before = std::find(m_pressed_keys_last_frame.begin(), m_pressed_keys_last_frame.end(), key) != m_pressed_keys_last_frame.end();

            if (pressed_now && !pressed_before) {
                const char* ev_name = action.c_str();
                InputEventType ev_type = InputEventType::PRESSED;
                float ev_axis_value = 0.0f;
                InputEvent event = InputEvent(ev_name, ev_type, ev_axis_value);

                dispatch_event(event);
            }
            else if (!pressed_now && pressed_before) {
                const char* ev_name = action.c_str();
                InputEventType ev_type = InputEventType::RELEASED;
                float ev_axis_value = 0.0f;
                InputEvent event = InputEvent(ev_name, ev_type, ev_axis_value);

                dispatch_event(event);
            }
        }
    }

    // Dispatch axis events
    for (auto& [axis, mapping] : m_input_mappings.axes) {
        bool any_positive = std::any_of(
            mapping.positive.begin(), mapping.positive.end(), [&](SDL_Scancode k) {
                return std::find(m_pressed_keys_this_frame.begin(), m_pressed_keys_this_frame.end(), k) != m_pressed_keys_this_frame.end();
            });

        bool any_negative = std::any_of(
            mapping.negative.begin(), mapping.negative.end(), [&](SDL_Scancode k) {
                return std::find(m_pressed_keys_this_frame.begin(), m_pressed_keys_this_frame.end(), k) != m_pressed_keys_this_frame.end();
            });

        float& axis_value = m_axis_values[axis];
        float old_axis_value = axis_value;

        if ((any_positive && any_negative) || (!any_positive && !any_negative)) {
            if (axis_value < 0.0f) {
                axis_value = std::min(axis_value + mapping.deceleration * delta_time, 0.0f);
            }
            else if (axis_value > 0.0f) {
                axis_value = std::max(axis_value - mapping.deceleration * delta_time, 0.0f);
            }
        }
        else if (any_positive) {
            axis_value = std::min(axis_value + mapping.acceleration * delta_time, 1.0f);
        }
        else if (any_negative) {
            axis_value = std::max(axis_value - mapping.acceleration * delta_time, -1.0f);
        }

        if (old_axis_value != axis_value) {
            const char* ev_name = axis.c_str();
            InputEventType ev_type = InputEventType::AXIS;
            float ev_axis_value = axis_value;
            InputEvent event = InputEvent(ev_name, ev_type, ev_axis_value);

            dispatch_event(event);
        }
    }
}

InputEventHandlerId Input::add_input_event_handler(InputEventHandler handler) {
    InputEventHandlerId handler_id = m_next_handler_id;
    m_next_handler_id += 1;

    m_handler_index_by_id[handler_id] = m_handlers.size();
    m_handlers.emplace_back(handler_id, std::move(handler));

    return handler_id;
}

bool Input::remove_input_event_handler(InputEventHandlerId id) {
    auto it = m_handler_index_by_id.find(id);
    if (it == m_handler_index_by_id.end()) {
        return false;
    }

    uint32_t index = it->second;
    uint32_t last_index = m_handlers.size() - 1;

    if (index != last_index) {
        m_handlers[index] = std::move(m_handlers[last_index]); // move last into hole
        m_handler_index_by_id[m_handlers[index].handler_id] = index; // fix moved id's index
    }

    m_handlers.pop_back();
    m_handler_index_by_id.erase(it);
    return true;
}

void Input::dispatch_event(const InputEvent& event) const {
    for (const auto& entry : m_handlers) {
        entry.handler(event);
    }
}

void Input::update_pressed_keys(const Uint8* keyboard_state) {
    m_pressed_keys_last_frame.clear();
    for (SDL_Scancode key : m_pressed_keys_this_frame) {
        m_pressed_keys_last_frame.push_back(key);
    }

    m_pressed_keys_this_frame.clear();
    for (SDL_Scancode key : m_relevant_keys) {
        if (keyboard_state[key]) {
            m_pressed_keys_this_frame.push_back(key);
        }
    }
}
