#include "debug.h"

#include <cmath>
#include <vector>

#include "engine/components/camera_component.h"
#include "engine/math/constants.h"
#include "systems/renderer.h"

namespace hob::debug {
    namespace {
        std::vector<DebugLine> lines;
        std::vector<DebugCircle> circles;

        void gl_draw_line(Renderer& renderer, const CameraComponent* camera, const DebugLine& line) {
            Vector2 a = camera->world_to_screen(line.start);
            Vector2 b = camera->world_to_screen(line.end);
            renderer.draw_line(a, b, line.color, line.thickness);
        }

        void gl_draw_circle(Renderer& renderer, const CameraComponent* camera, const DebugCircle& circle) {
            Vector2 prev_point = circle.center + Vector2(circle.radius, 0.0f);
            for (int i = 1; i <= circle.segments; ++i) {
                float ratio = static_cast<float>(i) / static_cast<float>(circle.segments);
                float angle = ratio * 2.0f * PI;
                Vector2 point = circle.center + Vector2(std::cos(angle), std::sin(angle)) * circle.radius;
                DebugLine line{prev_point, point, circle.color, 0.0f, circle.thickness};
                gl_draw_line(renderer, camera, line);

                prev_point = point;
            }
        }
    }

    void draw_debug_draws(Renderer& renderer, const CameraComponent* camera, float delta_time) {
        for (const auto& line : lines) {
            gl_draw_line(renderer, camera, line);
        }

        for (const auto& circle : circles) {
            gl_draw_circle(renderer, camera, circle);
        }

        std::erase_if(lines, [delta_time](DebugLine& l) {
            l.duration -= delta_time;
            return l.duration <= 0.0f;
        });

        std::erase_if(circles, [delta_time](DebugCircle& c) {
            c.duration -= delta_time;
            return c.duration <= 0.0f;
        });
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
