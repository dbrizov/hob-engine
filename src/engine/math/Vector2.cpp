#include "Vector2.h"

#include <cassert>
#include <fmt/format.h>

#include "Math.h"


Vector2::Vector2()
    : Vector2(0.0f, 0.0f) {
}

Vector2::Vector2(float x_, float y_)
    : x(x_)
      , y(y_) {
}

float Vector2::length() const {
    return sqrtf(x * x + y * y);
}

float Vector2::length_sqr() const {
    return x * x + y * y;
}

Vector2 Vector2::normalized() const {
    float len = length();
    if (len <= EPSILON) {
        return Vector2::zero();
    }

    return Vector2(x / len, y / len);
}

Vector2 Vector2::operator+(const Vector2& right) const {
    return Vector2(x + right.x, y + right.y);
}

Vector2& Vector2::operator+=(const Vector2& right) {
    x = x + right.x;
    y = y + right.y;
    return *this;
}

Vector2 Vector2::operator-() const {
    return Vector2(-x, -y);
}

Vector2 Vector2::operator-(const Vector2& right) const {
    return Vector2(x - right.x, y - right.y);
}

Vector2& Vector2::operator-=(const Vector2& right) {
    x = x - right.x;
    y = y - right.y;
    return *this;
}

Vector2 Vector2::operator*(float scalar) const {
    return Vector2(x * scalar, y * scalar);
}

Vector2 Vector2::operator/(float scalar) const {
    assert(scalar != 0.0f && "Division by zero");
    return Vector2(x / scalar, y / scalar);
}

bool Vector2::operator==(const Vector2& right) const {
    return
        (std::abs(x - right.x) < EPSILON) &&
        (std::abs(y - right.y) < EPSILON);
}

bool Vector2::operator!=(const Vector2& right) const {
    return !operator==(right);
}

Vector2 Vector2::zero() {
    return Vector2();
}

Vector2 Vector2::one() {
    return Vector2(1.0f, 1.0f);
}

Vector2 Vector2::left() {
    return Vector2(-1.0f, 0.0f);
}

Vector2 Vector2::right() {
    return Vector2(1.0f, 0.0f);
}

Vector2 Vector2::up() {
    return Vector2(0.0f, -1.0f);
}

Vector2 Vector2::down() {
    return Vector2(0.0f, 1.0f);
}

float Vector2::dot(const Vector2& a, const Vector2& b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 Vector2::lerp(const Vector2& a, const Vector2& b, float t) {
    return a * (1.0f - t) + b * t;
}

Vector2 Vector2::rotate_around(const Vector2& point, const Vector2& center, float degrees) {
    float rad = degrees * DEG_TO_RAD;
    float cos = std::cos(rad);
    float sin = std::sin(rad);

    Vector2 vec = point - center; // translate to origin

    Vector2 vec_r;
    vec_r.x = vec.x * cos - vec.y * sin;
    vec_r.y = vec.x * sin + vec.y * cos;

    return center + vec_r; // translate back
}

std::string Vector2::to_string() const {
    return fmt::format("({:.2f}, {:.2f})", x, y);
}
