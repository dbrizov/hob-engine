#include "Debug.h"

#include <SDL_render.h>
#include <cassert>

#include "engine/components/CameraComponent.h"
#include "engine/math/Math.h"

namespace debug {
    // Private
    static std::vector<DebugLine> lines;
    static SDL_Texture* white_pixel = nullptr;

    static void clear();
    static void sdl_draw_line(SDL_Renderer* renderer, const CameraComponent* camera, const DebugLine& line);

    // Public
    void render_debug_draws(SDL_Renderer* renderer, const CameraComponent* camera) {
        for (const auto& line : lines) {
            sdl_draw_line(renderer, camera, line);
        }

        clear();
    }

    void draw_line(const Vector2& start, const Vector2& end, const Color& color, float thickness/* = 1.0f */) {
        lines.emplace_back(start, end, color, thickness);
    }

    // Private (implementation)
    static void clear() {
        lines.clear();
    }

    static SDL_Texture* get_white_pixel(SDL_Renderer* renderer) {
        if (white_pixel == nullptr) {
            white_pixel = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STATIC,
                1, 1);

            assert(white_pixel != nullptr && "Could not create SDL texture for debug draw context");

            uint32_t pixel = 0xFFFFFFFF; // White RGBA
            SDL_UpdateTexture(white_pixel, nullptr, &pixel, sizeof(pixel));
        }

        return white_pixel;
    }

    static void sdl_draw_line(SDL_Renderer* renderer, const CameraComponent* camera, const DebugLine& line) {
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

        SDL_Texture* pixel = get_white_pixel(renderer);
        SDL_SetTextureColorMod(pixel, line.color.r, line.color.g, line.color.b);
        SDL_SetTextureAlphaMod(pixel, line.color.a);
        SDL_SetTextureBlendMode(pixel, SDL_BLENDMODE_BLEND);

        SDL_RenderCopyExF(renderer, pixel, nullptr, &dst, angle, &pivot, SDL_FLIP_NONE);
    }
}
