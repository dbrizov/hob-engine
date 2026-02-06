#ifndef CPP_PLATFORMER_VECTOR2_H
#define CPP_PLATFORMER_VECTOR2_H
#include <string>


struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    Vector2();
    Vector2(float x_, float y_);

    float length() const;
    float length_sqr() const;
    Vector2 normalized() const;

    Vector2 operator+(const Vector2& right) const;
    Vector2& operator+=(const Vector2& right);

    Vector2 operator-() const;
    Vector2 operator-(const Vector2& right) const;
    Vector2& operator-=(const Vector2& right);

    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;

    bool operator==(const Vector2& right) const;
    bool operator!=(const Vector2& right) const;

    static Vector2 zero();
    static Vector2 one();
    static Vector2 left();
    static Vector2 right();
    static Vector2 up();
    static Vector2 down();

    static float dot(const Vector2& a, const Vector2& b);
    static Vector2 lerp(const Vector2& a, const Vector2& b, float t);

    std::string to_string() const;
};


#endif //CPP_PLATFORMER_VECTOR2_H
