#pragma once
namespace Vector
{
    struct Vector3
    {
        float x, y, z;
    };

    inline Vector3 operator*(const float &a, const Vector3 &b)
    {
        Vector3 result = {};

        result.x = a * b.x;
        result.y = a * b.y;
        result.z = a * b.z;

        return result;
    }

    inline Vector3 operator*(const Vector3 &b, const float a)
    {
        Vector3 result = a * b;
        return result;
    }

    inline Vector3 &operator*=(Vector3 &b, const float a)
    {
        b = a * b;
        return b;
    }

    inline Vector3 operator/(const Vector3 &b, const float a)
    {
        Vector3 result = (1.0f / a) * b;
        return result;
    }

    inline Vector3 &operator/=(Vector3 &b, const float a)
    {
        b = b / a;
        return b;
    }

    inline Vector3 operator-(const Vector3 &a)
    {
        Vector3 result = {};

        result.x = -a.x;
        result.y = -a.y;
        result.z = -a.z;

        return result;
    }

    inline Vector3 operator+(const Vector3 &a, const Vector3 &b)
    {
        Vector3 result = {};

        result.x = a.x + b.x;
        result.y = a.y + b.y;
        result.z = a.z + b.z;

        return result;
    }

    inline Vector3 &operator+=(Vector3 &a, const Vector3 &b)
    {
        a = a + b;
        return a;
    }

    inline Vector3 operator-(const Vector3 &a, const Vector3 &b)
    {
        Vector3 result = {};

        result.x = a.x - b.x;
        result.y = a.y - b.y;
        result.z = a.z - b.z;

        return result;
    }

    inline Vector3 &operator-=(Vector3 &a, const Vector3 &b)
    {
        a = a - b;
        return a;
    }
} // namespace Vector