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

Vector2 Vector2::operator*(float a) const {
    return Vector2(x * a, y * a);
}

Vector2 Vector2::operator/(float a) const {
    assert(a == 0.0f);
    return Vector2(x / a, y / a);
}

bool Vector2::operator==(const Vector2& right) const {
    return
        (std::abs(x - right.x) < EPSILON) &&
        (std::abs(y - right.y) < EPSILON);
}

bool Vector2::operator!=(const Vector2& target) const {
    return !operator==(target);
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

float Vector2::dot(Vector2 a, Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

std::string Vector2::to_string() {
    return fmt::format("({:.2f}, {:.2f})", x, y);
}
