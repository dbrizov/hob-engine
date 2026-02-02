#include "Timer.h"

#include <SDL_timer.h>


Timer::Timer(uint32_t fps)
    : m_fps(fps)
      , m_time_scale(1.0f)
      , m_play_time(0.0f)
      , m_delta_time(0.0f) {
    m_frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    m_last_counter = SDL_GetPerformanceCounter();
}

void Timer::tick() {
    const double target_sec = 1.0 / static_cast<double>(m_fps);

    uint64_t now_ms = SDL_GetPerformanceCounter();
    double dt_sec = static_cast<double>(now_ms - m_last_counter) / m_frequency;

    // Wait until we reach target frame time
    while (dt_sec < target_sec) {
        double remaining = target_sec - dt_sec;

        // Sleep if we have a decent chunk left (avoid oversleeping too much)
        if (remaining > 0.002) {
            // 2ms
            SDL_Delay(static_cast<uint32_t>((remaining - 0.001) * 1000.0));
        }

        now_ms = SDL_GetPerformanceCounter();
        dt_sec = static_cast<double>(now_ms - m_last_counter) / m_frequency;
    }

    // After stalls (debugger, window focus loss, OS scheduling),
    // dt can become very large. Clamp it to avoid excessive physics
    // catch-up and the "spiral of death".
    m_last_counter = now_ms;
    if (dt_sec > 0.25) {
        dt_sec = 0.25;
    }

    m_delta_time = static_cast<float>(dt_sec);
    m_play_time += m_delta_time;
}

uint32_t Timer::get_fps() const {
    return m_fps;
}

void Timer::set_fps(uint32_t fps) {
    m_fps = fps;
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
