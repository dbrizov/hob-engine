#include "Debug.h"

#include <SDL_render.h>

#include "engine/components/CameraComponent.h"
#include "engine/math/Math.h"

namespace debug {
    // Private
    static std::vector<DebugLine> lines;

    static void clear();
    static void sdl_draw_line(SDL_Renderer* renderer, const CameraComponent* camera, const DebugLine& line);

    // Public
    void render_debug_draws(SDL_Renderer* renderer, const CameraComponent* camera_component) {
        for (const auto& line : lines) {
            sdl_draw_line(renderer, camera_component, line);
        }

        clear();
    }

    void draw_line(const Vector2& start, const Vector2& end, const Color& color, int thickness/* = 1 */) {
        lines.emplace_back(start, end, color, thickness);
    }

    // Private (implementation)
    static void clear() {
        lines.clear();
    }

    static void sdl_draw_line(SDL_Renderer* renderer, const CameraComponent* camera, const DebugLine& line) {
        Vector2 line_start = camera->world_to_screen(line.start);
        Vector2 line_end = camera->world_to_screen(line.end);

        Vector2 vec = line_end - line_start;
        float length = vec.length();
        if (length < EPSILON) {
            return;
        }

        // Normal vector (perpendicular)
        Vector2 normal = Vector2(-vec.y / length, vec.x / length);
        int thickness = std::max(1, line.thickness);
        int half = thickness / 2;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, line.color.r, line.color.g, line.color.b, line.color.a);

        for (int i = -half; i <= half; ++i) {
            Vector2 offset = normal * i;
            Vector2 from = line_start + offset;
            Vector2 to = line_end + offset;

            SDL_RenderDrawLineF(renderer, from.x, from.y, to.x, to.y);
        }
    }
}
