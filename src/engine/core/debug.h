#ifndef HOB_ENGINE_DEBUG_H
#define HOB_ENGINE_DEBUG_H
#include "render.h"
#include "engine/math/vector2.h"

struct SDL_Renderer;

namespace hob {
    class CameraComponent;

    namespace debug {
        struct DebugLine {
            Vector2 start;
            Vector2 end;
            Color color;
            float thickness = 1.0f;
        };

        struct DebugCircle {
            Vector2 center;
            float radius = 0.5f;
            Color color;
            float thickness = 1.0f;
            int segments = 16;
        };

        void render_debug_draws(SDL_Renderer* renderer,
                                SDL_Texture* white_pixel_texture,
                                const CameraComponent* camera);

        void draw_line(const Vector2& start,
                       const Vector2& end,
                       const Color& color,
                       float thickness = 1.0f);

        void draw_circle(const Vector2& center,
                         float radius,
                         const Color& color,
                         float thickness = 1.0f,
                         int segments = 16);
    }
}

#endif //HOB_ENGINE_DEBUG_H
