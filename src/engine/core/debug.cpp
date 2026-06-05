#include "debug.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "engine/components/camera_component.h"
#include "engine/math/constants.h"
#include "systems/renderer/renderer.h"

namespace hob::debug {
    namespace {
        std::vector<DebugLine> lines;
        std::vector<DebugCircle> circles;
        std::vector<DebugMessage> messages;

        constexpr float MESSAGE_MARGIN_X = 8.0f;
        constexpr float MESSAGE_MARGIN_Y = 8.0f;
        constexpr float MESSAGE_FADE_DURATION = 0.5f;
        constexpr size_t MAX_ON_SCREEN_MESSAGES = 32;

        float window_pixel_thickness(float thickness, const Vector2& window_size, const Vector2& logical_size) {
            const float ratio_x = (window_size.x > 0.0f) ? (logical_size.x / window_size.x) : 1.0f;
            const float ratio_y = (window_size.y > 0.0f) ? (logical_size.y / window_size.y) : 1.0f;
            // Average the two axes so thickness is isotropic when the window's aspect drifts
            // away from the logical aspect.
            const float ratio = 0.5f * (ratio_x + ratio_y);
            return thickness * ratio;
        }

        void r_draw_line(Renderer& renderer,
                         const CameraComponent* camera,
                         const Vector2& window_size,
                         const Vector2& logical_size,
                         const DebugLine& line) {
            Vector2 start = camera->world_to_screen(line.start);
            Vector2 end = camera->world_to_screen(line.end);
            const float thickness = window_pixel_thickness(line.thickness, window_size, logical_size);
            renderer.draw_debug_line(start, end, line.color, thickness);
        }

        void r_draw_circle(Renderer& renderer,
                           const CameraComponent* camera,
                           const Vector2& window_size,
                           const Vector2& logical_size,
                           const DebugCircle& circle) {
            Vector2 prev_point = circle.center + Vector2(circle.radius, 0.0f);
            for (int i = 1; i <= circle.segments; ++i) {
                float ratio = static_cast<float>(i) / static_cast<float>(circle.segments);
                float angle = ratio * 2.0f * PI;
                Vector2 point = circle.center + Vector2(std::cos(angle), std::sin(angle)) * circle.radius;
                DebugLine line{prev_point, point, circle.color, 0.0f, circle.thickness};
                r_draw_line(renderer, camera, window_size, logical_size, line);

                prev_point = point;
            }
        }
    }

    void flush_draws_to_renderer(Renderer& renderer,
                                 const CameraComponent* camera,
                                 const Vector2& window_size,
                                 float delta_time) {
        const Vector2 logical_size = renderer.get_logical_size();

        for (const auto& line : lines) {
            r_draw_line(renderer, camera, window_size, logical_size, line);
        }

        for (const auto& circle : circles) {
            r_draw_circle(renderer, camera, window_size, logical_size, circle);
        }

        const float line_height = static_cast<float>(renderer.get_debug_font_line_height());
        float pen_y = MESSAGE_MARGIN_Y;
        for (const DebugMessage& msg : messages) {
            const float fade = std::clamp(msg.duration / MESSAGE_FADE_DURATION, 0.0f, 1.0f);
            Color c = msg.color;
            c.a *= fade;
            renderer.draw_debug_text(Vector2(MESSAGE_MARGIN_X, pen_y), msg.text, c);
            pen_y += line_height;
        }

        std::erase_if(lines, [delta_time](DebugLine& l) {
            l.duration -= delta_time;
            return l.duration <= 0.0f;
        });

        std::erase_if(circles, [delta_time](DebugCircle& c) {
            c.duration -= delta_time;
            return c.duration <= 0.0f;
        });

        std::erase_if(messages, [delta_time](DebugMessage& m) {
            m.duration -= delta_time;
            return m.duration <= 0.0f;
        });
    }

    void add_on_screen_message(std::string text, Color color, float duration) {
        if (messages.size() >= MAX_ON_SCREEN_MESSAGES) {
            messages.erase(messages.begin());
        }

        messages.push_back(DebugMessage{std::move(text), color, duration});
    }

    void draw_line(const Vector2& start,
                   const Vector2& end,
                   const Color& color,
                   float duration,
                   float thickness) {
        lines.emplace_back(start, end, color, duration, thickness);
    }

    void draw_circle(const Vector2& center,
                     float radius,
                     const Color& color,
                     float duration,
                     float thickness,
                     int segments) {
        circles.emplace_back(center, radius, color, duration, thickness, segments);
    }
}
