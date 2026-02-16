#include "Debug.h"

#include <SDL_render.h>

#include "engine/components/CameraComponent.h"
#include "engine/math/Math.h"

namespace debug {
    namespace {
        std::vector<DebugLine> lines;
        std::vector<DebugCircle> circles;

        void sdl_draw_line(SDL_Renderer* renderer,
                           SDL_Texture* white_pixel_texture,
                           const CameraComponent* camera,
                           const DebugLine& line) {
            // Transform to screen space first
            Vector2 a = camera->world_to_screen(line.start);
            Vector2 b = camera->world_to_screen(line.end);

            Vector2 v = b - a;
            float length = v.length();
            if (length < EPSILON) {
                return;
            }

            float angle = std::atan2(v.y, v.x) * RAD_TO_DEG; // Angle in degrees for SDL
            float thickness = std::max(1.0f, line.thickness);

            // Destination rectangle:
            // width = line length
            // height = thickness
            SDL_FRect dst;
            dst.x = a.x;
            dst.y = a.y - thickness * 0.5f;
            dst.w = length;
            dst.h = thickness;

            // Pivot so the rectangle rotates around the start point
            SDL_FPoint pivot;
            pivot.x = 0.0f;
            pivot.y = thickness * 0.5f;

            SDL_SetTextureColorMod(white_pixel_texture, line.color.r, line.color.g, line.color.b);
            SDL_SetTextureAlphaMod(white_pixel_texture, line.color.a);
            SDL_SetTextureBlendMode(white_pixel_texture, SDL_BLENDMODE_BLEND);

            SDL_RenderCopyExF(renderer, white_pixel_texture, nullptr, &dst, angle, &pivot, SDL_FLIP_NONE);
        }

        void sdl_draw_circle(SDL_Renderer* renderer,
                             SDL_Texture* white_pixel_texture,
                             const CameraComponent* camera,
                             const DebugCircle& circle) {
            Vector2 prev_point = circle.center + Vector2(circle.radius, 0.0f);
            for (int i = 1; i <= circle.segments; ++i) {
                float ratio = static_cast<float>(i) / static_cast<float>(circle.segments);
                float angle = ratio * 2.0f * PI;
                Vector2 point = circle.center + Vector2(std::cos(angle), std::sin(angle)) * circle.radius;
                DebugLine line{prev_point, point, circle.color, circle.thickness};
                sdl_draw_line(renderer, white_pixel_texture, camera, line);

                prev_point = point;
            }
        }
    }

    void render_debug_draws(SDL_Renderer* renderer, SDL_Texture* white_pixel_texture, const CameraComponent* camera) {
        for (const auto& line : lines) {
            sdl_draw_line(renderer, white_pixel_texture, camera, line);
        }

        for (const auto& circle : circles) {
            sdl_draw_circle(renderer, white_pixel_texture, camera, circle);
        }

        lines.clear();
        circles.clear();
    }

    void draw_line(const Vector2& start, const Vector2& end, const Color& color, float thickness) {
        lines.emplace_back(start, end, color, thickness);
    }

    void draw_circle(const Vector2& center, float radius, const Color& color, float thickness, int segments) {
        circles.emplace_back(center, radius, color, thickness, segments);
    }
}
