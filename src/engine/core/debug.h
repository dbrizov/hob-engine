#pragma once

#include "engine/math/color.h"
#include "engine/math/vector2.h"

namespace hob {
    class CameraComponent;
    class Renderer;

    namespace debug {
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
    }
}
