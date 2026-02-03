#ifndef CPP_PLATFORMER_INPUT_H
#define CPP_PLATFORMER_INPUT_H
#include <filesystem>
#include <functional>
#include <SDL_scancode.h>
#include <string>
#include <unordered_map>
#include <vector>


enum class InputEventType {
    AXIS,
    PRESSED,
    RELEASED,
};


struct InputEvent {
    const char* name;
    InputEventType type;
    float axis_value;

    InputEvent(const char* ev_name, InputEventType ev_type, float ev_axis_value);
};


using InputEventHandlerId = int;
using InputEventHandler = std::function<void(const InputEvent&)>;


struct AxisMapping {
    float acceleration;
    float deceleration;
    std::vector<SDL_Scancode> positive;
    std::vector<SDL_Scancode> negative;
};


struct InputMappings {
    std::unordered_map<std::string, std::vector<SDL_Scancode>> actions;
    std::unordered_map<std::string, AxisMapping> axes;

    std::vector<SDL_Scancode> relevant_keys() const;
};


class Input {
    struct HandlerEntry {
        InputEventHandlerId handler_id;
        InputEventHandler handler;
    };

    InputEventHandlerId m_next_handler_id = 0;
    std::vector<HandlerEntry> m_handlers;
    std::unordered_map<InputEventHandlerId, uint32_t> m_handler_index_by_id;

    InputMappings m_input_mappings;
    std::unordered_map<std::string, float> m_axis_values;
    std::vector<SDL_Scancode> m_relevant_keys;
    std::vector<bool> m_pressed_keys_this_frame;
    std::vector<bool> m_pressed_keys_last_frame;

public:
    explicit Input(const std::filesystem::path& input_config_path);

    void tick(float delta_time, const uint8_t* keyboard_state);

    InputEventHandlerId add_input_event_handler(InputEventHandler handler);
    bool remove_input_event_handler(InputEventHandlerId id);

private:
    void dispatch_event(const InputEvent& event) const;
    void update_pressed_keys(const uint8_t* keyboard_state);
};


#endif //CPP_PLATFORMER_INPUT_H
