#pragma once
#include <cmath>

namespace SquareCore
{
    inline float Lerp(float a, float b, float alpha) {
        return a + alpha * (b - a);
    }

    inline float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    inline float Abs(float value) {
        return value >= 0.0f ? value : -value;
    }

    inline float Min(float a, float b) {
        return a < b ? a : b;
    }

    inline float Max(float a, float b) {
        return a > b ? a : b;
    }

    inline bool CompareFloats(float a, float b, float epsilon = 0.0001f)
    {
        return std::abs(a-b) < epsilon;
    }
}
