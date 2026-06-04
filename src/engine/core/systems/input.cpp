#include "input.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

#include "engine/core/path_utils.h"
#include "renderer/renderer.h"
#include "sdl_context.h"

namespace hob {
    InputEvent::InputEvent(const char* ev_name, InputEventType ev_type, float ev_axis_value)
        : name(ev_name)
        , type(ev_type)
        , axis_value(ev_axis_value) {
    }

    Input::Input(const SdlContext& sdl_context, const Renderer& renderer)
        : m_sdl_context(sdl_context)
        , m_renderer(renderer) {
        m_input_config = InputConfig(PathUtils::get_input_config_path());
        m_relevant_keys = m_input_config.relevant_keys();

        for (const auto& [axis, _] : m_input_config.axes) {
            m_axis_values[axis] = 0.0f;
        }
    }

    void Input::tick(float delta_time) {
        update_mouse_screen_position();
        update_pressed_keys();

        // Dispatch action events
        for (auto& [action, mapping] : m_input_config.actions) {
            for (auto key : mapping.keys) {
                bool pressed_now = m_pressed_keys_this_frame.test(key);
                bool pressed_before = m_pressed_keys_last_frame.test(key);

                if (pressed_now && !pressed_before) {
                    dispatch_event(InputEvent(action.c_str(), InputEventType::Pressed, 0.0f));
                }
                else if (!pressed_now && pressed_before) {
                    dispatch_event(InputEvent(action.c_str(), InputEventType::Released, 0.0f));
                }
            }
        }

        // Dispatch axis events
        auto any_pressed = [&](const auto& keys) {
            for (SDL_Scancode key : keys) {
                if (m_pressed_keys_this_frame.test(key)) {
                    return true;
                }
            }

            return false;
        };

        for (auto& [axis, mapping] : m_input_config.axes) {
            bool any_positive = any_pressed(mapping.positive_keys);
            bool any_negative = any_pressed(mapping.negative_keys);

            float& axis_value = m_axis_values[axis];

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

            dispatch_event(InputEvent(axis.c_str(), InputEventType::Axis, axis_value));
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

        size_t index = it->second;
        size_t last_index = m_handlers.size() - 1;

        if (index != last_index) {
            m_handlers[index] = std::move(m_handlers[last_index]); // move last into hole
            m_handler_index_by_id[m_handlers[index].handler_id] = index; // fix moved id's index
        }

        m_handlers.pop_back();
        m_handler_index_by_id.erase(it);
        return true;
    }

    Vector2 Input::get_mouse_screen_position() const {
        return m_mouse_screen_position;
    }

    void Input::update_mouse_screen_position() {
        float x = 0.0f;
        float y = 0.0f;
        SDL_GetMouseState(&x, &y);

        // Map window pixels to logical (FBO) pixels. The FBO is blitted to the window with a
        // uniform STRETCH (no letterboxing today), so the mapping is just an axis-wise scale.
        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSize(m_sdl_context.get_window(), &window_w, &window_h);

        const float logical_w = m_renderer.get_logical_width_f();
        const float logical_h = m_renderer.get_logical_height_f();

        if (window_w > 0 && window_h > 0) {
            x = x * (logical_w / static_cast<float>(window_w));
            y = y * (logical_h / static_cast<float>(window_h));
        }

        m_mouse_screen_position = Vector2(x, y);
    }

    void Input::update_pressed_keys() {
        const bool* keyboard_state = SDL_GetKeyboardState(nullptr);
        for (SDL_Scancode key : m_relevant_keys) {
            m_pressed_keys_last_frame.set(key, m_pressed_keys_this_frame.test(key));
            m_pressed_keys_this_frame.set(key, keyboard_state[key]);
        }
    }

    void Input::dispatch_event(const InputEvent& event) const {
        for (const auto& entry : m_handlers) {
            entry.handler(event);
        }
    }
}
