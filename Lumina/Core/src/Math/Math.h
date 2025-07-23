#pragma once
#include "Core/Base.h"

namespace Lumina::Math
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float HALF_PI = 0.5f * PI;
    constexpr float DEG2RAD = PI / 180.0f;
    constexpr float RAD2DEG = 180.0f / PI;

    float Radians(float degrees);
    float Degrees(float radians);
    float Lerp(float a, float b, float t);
    float Clamp(float value, float min, float max);
    float Saturate(float value);
    float Smoothstep(float edge0, float edge1, float x);
}