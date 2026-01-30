#ifndef CPP_PLATFORMER_TIME_H
#define CPP_PLATFORMER_TIME_H
#include <cstdint>


class Timer {
private:
    uint32_t m_fps;
    float m_time_scale;
    float m_play_time;
    float m_delta_time;
    uint32_t m_last_ticks;

public:
    explicit Timer(uint32_t fps);

    void tick();

    uint32_t get_fps() const;
    void set_fps(uint32_t fps);

    float get_time_scale() const;
    void set_time_scale(float time_scale);

    float get_play_time() const;
    void set_play_time(float play_time);

    float get_delta_time() const;
};


#endif //CPP_PLATFORMER_TIME_H
