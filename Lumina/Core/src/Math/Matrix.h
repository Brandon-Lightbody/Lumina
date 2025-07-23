#pragma once
#include "Core/Base.h"

namespace Lumina::Math
{
    struct Matrix4
    {
        float m[4][4];

        static Matrix4 Identity();
        static Matrix4 Translate(const Vector3& translation);
        static Matrix4 Rotate(float angle, const Vector3& axis);
        static Matrix4 Scale(const Vector3& scale);
        static Matrix4 Perspective(float fov, float aspect, float near, float far);

        Matrix4 operator*(const Matrix4& other) const;
    };
}