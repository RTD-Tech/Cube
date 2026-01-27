#ifndef MATH_H
#define MATH_H

#include "MathFunctions.h"
#include <cmath>
#include <cstdio>

#define MATH_PI 3.14159265358979323846f
#define MATH_TAU (MATH_PI * 2.0)

namespace SquareCore {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    // Vector operations
    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/(float scalar) const {
        return Vec2(x / scalar, y / scalar);
    }

    // Assignment operators
    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Vector properties
    float magnitude() const {
        return sqrt(x * x + y * y);
    }

    float magnitudeSquared() const {
        return x * x + y * y;
    }

    Vec2 normalized() const {
        float mag = magnitude();
        if (mag > 0.0f) {
            return Vec2(x / mag, y / mag);
        }
        return Vec2(0.0f, 0.0f);
    }

    void normalize() {
        float mag = magnitude();
        if (mag > 0.0f) {
            x /= mag;
            y /= mag;
        }
    }

    float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }

    float distance(const Vec2& other) const {
        return (*this - other).magnitude();
    }

    float distanceSquared(const Vec2& other) const {
        return (*this - other).magnitudeSquared();
    }
    
    Vec2 perpendicular() const {
        return Vec2(-y, x);
    }
    
    Vec2 reflect(const Vec2& normal) const {
        return *this - normal * (2.0f * this->dot(normal));
    }
    
    static Vec2 lookAt(const Vec2& from, const Vec2& to) {
        return (to - from).normalized();
    }
    
    static Vec2 lerp(const Vec2& a, const Vec2& b, float alpha) {
        return Vec2(Lerp(a.x, b.x, alpha), Lerp(a.y, b.y, alpha));
    }
    
    static Vec2 slerp(const Vec2& a, const Vec2& b, float alpha) {
        float magA = a.magnitude();
        float magB = b.magnitude();
        
        Vec2 normA = a.normalized();
        Vec2 normB = b.normalized();
        
        float dotProduct = Clamp(normA.dot(normB), -1.0f, 1.0f);
        float theta = acos(dotProduct) * alpha;
        
        Vec2 relative = normB - normA * dotProduct;
        relative.normalize();
        
        float mag = magA + alpha * (magB - magA);
        
        return (normA * cos(theta) + relative * sin(theta)) * mag;
    }

    // Static utility functions
    static Vec2 zero() { return Vec2(0.0f, 0.0f); }
    static Vec2 one() { return Vec2(1.0f, 1.0f); }
    static Vec2 up() { return Vec2(0.0f, 1.0f); }
    static Vec2 down() { return Vec2(0.0f, -1.0f); }
    static Vec2 left() { return Vec2(-1.0f, 0.0f); }
    static Vec2 right() { return Vec2(1.0f, 0.0f); }
    
    char* ToString() const {
        static char buffer[64];
        snprintf(buffer, sizeof(buffer), "{%.1f, %.1f}", x, y);
        return buffer;
    }
};

}

#endif
