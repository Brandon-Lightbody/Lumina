#pragma once
#include "Core/Base.h"

namespace Lumina::Math
{
    struct Vector2
    {
        float x, y;

        Vector2() : x(0), y(0) {}
        Vector2(float x, float y) : x(x), y(y) {}

        static Vector2 Zero() { return { 0, 0 }; }
        static Vector2 One() { return { 1, 1 }; }

        Vector2 operator+(const Vector2& other) const;
        Vector2 operator-(const Vector2& other) const;
        Vector2 operator*(float scalar) const;
    };

    struct Vector3
    {
        float x, y, z;

        Vector3() : x(0), y(0), z(0) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        static Vector3 Zero() { return { 0, 0, 0 }; }
        static Vector3 One() { return { 1, 1, 1 }; }
        static Vector3 Up() { return { 0, 1, 0 }; }
        static Vector3 Forward() { return { 0, 0, 1 }; }
        static Vector3 Right() { return { 1, 0, 0 }; }

        Vector3 operator+(const Vector3& other) const;
        Vector3 operator-(const Vector3& other) const;
        Vector3 operator*(float scalar) const;
        float Dot(const Vector3& other) const;
        Vector3 Cross(const Vector3& other) const;
        float Length() const;
        Vector3 Normalized() const;
    };

    struct Vector4
    {
        float x, y, z, w;

        Vector4() : x(0), y(0), z(0), w(0) {}
        Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

        static Vector4 Zero() { return { 0, 0, 0, 0 }; }
        static Vector4 One() { return { 1, 1, 1, 1 }; }
    };
}