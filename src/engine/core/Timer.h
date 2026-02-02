#ifndef CPP_PLATFORMER_TIME_H
#define CPP_PLATFORMER_TIME_H
#include <cstdint>


class Timer {
    uint32_t m_target_fps;
    bool m_vsync_enabled;
    float m_time_scale;
    float m_play_time;
    float m_delta_time;

    // Used for limiting FPS
    double m_frequency;
    uint64_t m_frame_start_counter;
    uint64_t m_last_counter;

public:
    Timer(uint32_t fps, bool vsync_enabled);

    void frame_start();
    void frame_end();

    uint32_t get_fps() const;
    void set_fps(uint32_t fps);

    float get_time_scale() const;
    void set_time_scale(float time_scale);

    float get_play_time() const;
    void set_play_time(float play_time);

    float get_delta_time() const;
};


#endif //CPP_PLATFORMER_TIME_H
