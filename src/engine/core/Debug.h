#ifndef CPP_PLATFORMER_DEBUG_H
#define CPP_PLATFORMER_DEBUG_H

#include "Render.h"
#include "engine/math/Vector2.h"


struct SDL_Renderer;
class CameraComponent;


namespace debug {
    struct DebugLine {
        Vector2 start;
        Vector2 end;
        Color color;
        float thickness = 1.0f;
    };

    void render_debug_draws(SDL_Renderer* renderer, const CameraComponent* camera);

    void draw_line(const Vector2& start, const Vector2& end, const Color& color, float thickness = 1.0f);
}


#endif //CPP_PLATFORMER_DEBUG_H