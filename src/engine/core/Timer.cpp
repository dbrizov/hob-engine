#include "Timer.h"

#include <SDL_timer.h>


Timer::Timer(uint32_t fps, bool vsync_enabled)
    : m_target_fps(fps)
      , m_vsync_enabled(vsync_enabled)
      , m_time_scale(1.0f)
      , m_play_time(0.0f)
      , m_delta_time(0.0f)
      , m_frequency(0.0f)
      , m_frame_start_counter(0)
      , m_last_counter(0) {
    m_frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    m_last_counter = SDL_GetPerformanceCounter();
}

void Timer::frame_start() {
    uint64_t now = SDL_GetPerformanceCounter();

    uint64_t diff = now - m_last_counter;
    m_last_counter = now;

    double dt_sec = static_cast<double>(diff) / m_frequency;

    // After stalls (debugger, window focus loss, OS scheduling),
    // dt can become very large. Clamp it to avoid excessive physics
    // catch-up and the "spiral of death".
    if (dt_sec > 0.25) {
        dt_sec = 0.25;
    }

    m_delta_time = static_cast<float>(dt_sec);
    m_play_time += m_delta_time;

    // Remember when this frame started (for frame_end)
    m_frame_start_counter = now;
}

void Timer::frame_end() {
    if (m_vsync_enabled || m_target_fps == 0) {
        return;
    }

    const double target_frame_sec = 1.0 / static_cast<double>(m_target_fps);

    while (true) {
        uint64_t now = SDL_GetPerformanceCounter();
        double elapsed_sec = static_cast<double>(now - m_frame_start_counter) / m_frequency;

        if (elapsed_sec >= target_frame_sec) {
            break;
        }

        double remaining_sec = target_frame_sec - elapsed_sec;

        // Sleep most of the remaining time (avoid oversleep)
        if (remaining_sec > 0.002) {
            // ~2ms
            SDL_Delay(static_cast<uint32_t>((remaining_sec - 0.001) * 1000.0));
        }
    }
}

uint32_t Timer::get_fps() const {
    return m_target_fps;
}

void Timer::set_fps(uint32_t fps) {
    m_target_fps = fps;
}

float Timer::get_time_scale() const {
    return m_time_scale;
}

void Timer::set_time_scale(float time_scale) {
    m_time_scale = time_scale;
}

float Timer::get_play_time() const {
    return m_play_time;
}

void Timer::set_play_time(float play_time) {
    m_play_time = play_time;
}

float Timer::get_delta_time() const {
    return m_delta_time;
}
