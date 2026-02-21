#ifndef HOB_ENGINE_INPUT_H
#define HOB_ENGINE_INPUT_H
#include <filesystem>
#include <functional>
#include <SDL_scancode.h>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

namespace hob {
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

    constexpr InputEventHandlerId INVALID_INPUT_EVENT_HANDLER_ID = -1;

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
        std::unordered_map<InputEventHandlerId, size_t> m_handler_index_by_id;

        InputMappings m_input_mappings;
        std::unordered_map<std::string, float> m_axis_values;
        std::vector<SDL_Scancode> m_relevant_keys;
        std::bitset<SDL_NUM_SCANCODES> m_pressed_keys_this_frame{};
        std::bitset<SDL_NUM_SCANCODES> m_pressed_keys_last_frame{};

    public:
        Input();

        void tick(float delta_time, const uint8_t* keyboard_state);

        InputEventHandlerId add_input_event_handler(InputEventHandler handler);
        bool remove_input_event_handler(InputEventHandlerId id);

    private:
        void dispatch_event(const InputEvent& event) const;
        void update_pressed_keys(const uint8_t* keyboard_state);
    };
}

#endif //HOB_ENGINE_INPUT_H
