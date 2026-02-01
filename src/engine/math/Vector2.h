#ifndef CPP_PLATFORMER_VECTOR2_H
#define CPP_PLATFORMER_VECTOR2_H
#include <cmath>

#include "Math.h"


struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;

    constexpr Vector2(float x_, float y_)
        : x(x_)
          , y(y_) {
    }

    static constexpr Vector2 zero() { return Vector2(); }
    static constexpr Vector2 one() { return Vector2(1.0f, 1.0f); }
    static constexpr Vector2 left() { return Vector2(-1.0f, 0.0f); }
    static constexpr Vector2 right() { return Vector2(1.0f, 0.0f); }
    static constexpr Vector2 up() { return Vector2(0.0f, -1.0f); }
    static constexpr Vector2 down() { return Vector2(0.0f, 1.0f); }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    constexpr float length_sqr() const {
        return x * x + y * y;
    }

    Vector2 normalized() const {
        const float len = length();
        if (len < EPSILON) {
            return Vector2::zero();
        }

        return Vector2(x / len, y / len);
    }

    static constexpr float dot(Vector2 a, Vector2 b) {
        return a.x * b.x + a.y * b.y;
    }

    constexpr Vector2& operator+=(Vector2 rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr Vector2& operator-=(Vector2 rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
};


constexpr Vector2 operator+(Vector2 a, Vector2 b) {
    return a += b;
}

constexpr Vector2 operator-(Vector2 a, Vector2 b) {
    return a -= b;
}

constexpr Vector2 operator*(Vector2 v, float s) {
    return v *= s;
}

constexpr Vector2 operator*(float s, Vector2 v) {
    return v *= s;
}

constexpr Vector2 operator/(Vector2 v, float s) {
    return v /= s;
}

constexpr Vector2 operator-(Vector2 v) {
    return Vector2(-v.x, -v.y);
}

inline bool operator==(Vector2 a, Vector2 b) {
    return
        std::abs(a.x - b.x) < EPSILON &&
        std::abs(a.y - b.y) < EPSILON;
}

inline bool operator!=(Vector2 a, Vector2 b) {
    return !(a == b);
}


#endif //CPP_PLATFORMER_VECTOR2_H
