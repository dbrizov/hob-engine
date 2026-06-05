#pragma once

#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include "engine/math/color.h"
#include "engine/math/vector2.h"

namespace hob {
    class CameraComponent;
    class Renderer;

    namespace debug {
        // --- Console logging ---

        template<typename... Args>
        void log(std::format_string<Args...> fmt, Args&&... args) {
            std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
        }

        template<typename... Args>
        void log_error(std::format_string<Args...> fmt, Args&&... args) {
            std::cerr << std::format(fmt, std::forward<Args>(args)...) << std::endl;
        }

        // --- Debug primitives ---

        struct DebugLine {
            Vector2 start;
            Vector2 end;
            Color color;
            float duration = 0.0f;
            float thickness = 1.0f;
        };

        struct DebugCircle {
            Vector2 center;
            float radius = 0.5f;
            Color color;
            float duration = 0.0f;
            float thickness = 1.0f;
            int segments = 16;
        };

        struct DebugMessage {
            std::string text;
            Color color;
            float duration = 0.0f;
        };

        void flush_draws_to_renderer(Renderer& renderer,
                                     const CameraComponent* camera,
                                     const Vector2& window_size,
                                     float delta_time);

        void draw_line(const Vector2& start,
                       const Vector2& end,
                       const Color& color,
                       float duration = 0.0f,
                       float thickness = 1.0f);

        void draw_circle(const Vector2& center,
                         float radius,
                         const Color& color,
                         float duration = 0.0f,
                         float thickness = 1.0f,
                         int segments = 16);

        // --- On-screen messages  ---

        // Non-template entry point; called by all print(...) overloads.
        void add_on_screen_message(std::string text, Color color, float duration);

        namespace detail {
            inline constexpr float DEFAULT_PRINT_DURATION = 2.0f;
            inline constexpr Color DEFAULT_PRINT_COLOR = Color::green();

            template<typename... Args>
            void print_dispatch(float duration, Color color, std::format_string<Args...> fmt, Args&&... args) {
                log(fmt, args...);
                add_on_screen_message(std::format(fmt, args...), color, duration);
            }
        }

        template<typename... Args>
        void print(std::format_string<Args...> fmt, Args&&... args) {
            detail::print_dispatch(detail::DEFAULT_PRINT_DURATION,
                                   detail::DEFAULT_PRINT_COLOR,
                                   fmt,
                                   std::forward<Args>(args)...);
        }

        template<typename... Args>
        void print(Color color, float duration, std::format_string<Args...> fmt, Args&&... args) {
            detail::print_dispatch(duration, color, fmt, std::forward<Args>(args)...);
        }
    }
}
