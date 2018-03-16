#pragma once
namespace Vector
{
    struct Vector3
    {
        float x, y, z;

        Vector3 &operator*=(const float &a)
        {
            x *= a;
            y *= a;
            z *= a;

            return *this;
        }

        Vector3 &operator/=(const float &a)
        {
            x *= (1.0f / a);
            y *= (1.0f / a);
            z *= (1.0f / a);

            return *this;
        }

        Vector3 &operator+=(const Vector3 &a)
        {
            x += a.x;
            y += a.y;
            z += a.z;

            return *this;
        }

        const Vector3 operator-() const
        {
            Vector3 result = {};

            result.x = -x;
            result.y = -y;
            result.z = -z;

            return result;
        }

        Vector3 &operator-=(const Vector3 &a)
        {
            x -= a.x;
            y -= a.y;
            z -= a.z;

            return *this;
        }
    };

    inline Vector3 operator*(const float &a, const Vector3 &b)
    {
        Vector3 result = {};

        result.x = a * b.x;
        result.y = a * b.y;
        result.z = a * b.z;

        return result;
    }

    inline Vector3 operator*(const Vector3 &b, const float &a)
    {
        Vector3 result = a * b;
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

    inline Vector3 operator-(const Vector3 &a, const Vector3 &b)
    {
        Vector3 result = {};

        result.x = a.x - b.x;
        result.y = a.y - b.y;
        result.z = a.z - b.z;

        return result;
    }
} // namespace Vector