#ifndef CPP_PLATFORMER_INPUTCOMPONENT_H
#define CPP_PLATFORMER_INPUTCOMPONENT_H
#include "Component.h"
#include "engine/core/Input.h"


using AxisBindingFunc = std::function<void(float)>;
using ActionBindingFunc = std::function<void()>;


class InputComponent : public Component {
    struct AxisBindingEntry {
        uint32_t id;
        AxisBindingFunc function;
    };

    struct ActionBindingEntry {
        uint32_t id;
        ActionBindingFunc function;
    };

    InputEventHandlerId m_input_event_handler_id = 0;

    uint32_t m_next_binding_id = 0;
    std::unordered_map<std::string, std::vector<AxisBindingEntry>> m_axis_bindings;
    std::unordered_map<std::string, std::vector<ActionBindingEntry>> m_action_pressed_bindings;
    std::unordered_map<std::string, std::vector<ActionBindingEntry>> m_action_released_bindings;

public:
    virtual ComponentPriority get_priority() const override;

    virtual void enter_play() override;
    virtual void exit_play() override;

    uint32_t bind_axis(const char* axis_name, AxisBindingFunc function);
    void unbind_axis(const char* axis_name, uint32_t axis_binding_id);

    uint32_t bind_action(const char* action_name, InputEventType event_type, ActionBindingFunc function);
    void unbind_action(const char* action_name, uint32_t action_binding_id);

    void clear_all_bindings();

private:
    void on_input_event(const InputEvent& event);
};


#endif //CPP_PLATFORMER_INPUTCOMPONENT_H
