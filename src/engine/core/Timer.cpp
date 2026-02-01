#include "Timer.h"

#include <SDL_timer.h>

Timer::Timer(uint32_t fps)
    : m_fps(fps)
      , m_time_scale(1.0f)
      , m_play_time(0.0f)
      , m_delta_time(0.0f)
      , m_last_ticks(0) {
}

void Timer::tick() {
    uint32_t current_ticks = SDL_GetTicks();
    uint32_t delta_ms = current_ticks - m_last_ticks;
    uint32_t target_ms = 1000 / m_fps;

    if (delta_ms < target_ms) {
        SDL_Delay(target_ms - delta_ms);

        uint32_t after_delay_ticks = SDL_GetTicks();
        delta_ms = after_delay_ticks - m_last_ticks;
    }

    m_last_ticks = SDL_GetTicks();
    m_delta_time = static_cast<float>(delta_ms) / 1000.0f;
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
