#pragma once
#include <cmath>
#include <algorithm>

#define LOMONT_CONSTANT 0x5f375a86
#define SQUARED_EPSILON 0.00000001f
#define LINEAR_CUTOFF 0.0031308f
#define SLOPE_HORIZONTAL 12.92f

namespace Math
{
    struct Vector3
    {
        float x, y, z;
    };

    inline Vector3 operator*(float A, Vector3 B)
    {
        Vector3 Result;

        Result.x = A * B.x;
        Result.y = A * B.y;
        Result.z = A * B.z;

        return (Result);
    }

    inline Vector3 operator*(Vector3 B, float A)
    {
        Vector3 Result = A * B;

        return (Result);
    }

    inline Vector3 &operator*=(Vector3 &B, float A)
    {
        B = A * B;

        return (B);
    }

    inline Vector3 operator/(Vector3 B, float A)
    {
        Vector3 Result = (1.0f / A) * B;

        return (Result);
    }

    inline Vector3 &operator/=(Vector3 &B, float A)
    {
        B = B / A;

        return (B);
    }

    inline Vector3 operator-(Vector3 A)
    {
        Vector3 Result;

        Result.x = -A.x;
        Result.y = -A.y;
        Result.z = -A.z;

        return (Result);
    }

    inline Vector3 operator+(Vector3 A, Vector3 B)
    {
        Vector3 Result;

        Result.x = A.x + B.x;
        Result.y = A.y + B.y;
        Result.z = A.z + B.z;

        return (Result);
    }

    inline Vector3 &operator+=(Vector3 &A, Vector3 B)
    {
        A = A + B;

        return (A);
    }

    inline Vector3 operator-(Vector3 A, Vector3 B)
    {
        Vector3 Result;

        Result.x = A.x - B.x;
        Result.y = A.y - B.y;
        Result.z = A.z - B.z;

        return (Result);
    }

    inline Vector3 &operator-=(Vector3 &A, Vector3 B)
    {
        A = A - B;

        return (A);
    }

    inline float square_root(float a)
    {
        auto result = sqrt(a);
        return result;
    }

    inline uint32_t round_float_to_uint32(float f)
    {
        uint32_t result = (uint32_t) (f + 0.5f);
        return result;
    }

    struct RandomSeries
    {
        uint32_t state;
    };

    inline uint32_t xor_shift(RandomSeries *series)
    {
        // xorshift reference needed
        uint32_t x = series->state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        series->state = x;
        return x;
    }

    float random_unilateral(RandomSeries *series)
    {
        float result = (float) xor_shift(series) / (float) UINT32_MAX;
        return result;
    }

    float random_bilateral(RandomSeries *series)
    {
        float result = -1.0f + 2.0f * random_unilateral(series);
        return result;
    }

    inline Vector3 hadamard_product(Vector3 a, Vector3 b)
    {
        Vector3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
        return result;
    }

    inline float inner_product(Vector3 a, Vector3 b)
    {
        float result = a.x * b.x + a.y * b.y + a.z * b.z;
        return result;
    }

    inline Vector3 cross_product(Vector3 a, Vector3 b)
    {
        Vector3 result;

        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;

        return result;
    }

    float inverse_sqrt(float x)
    {
        float x_half = 0.5f * x;
        union
        {
            float x;
            int i;
        } u;
        u.x = x;
        u.i = LOMONT_CONSTANT - (u.i >> 1);
        u.x = u.x * (1.5f - x_half * u.x * u.x);

        return u.x;
    }

    // automatic epsilon check
    // get length, if length is greater than epsilon, otherwise return zero
    inline Vector3 normalize_or_zero(Vector3 a)
    {
        Vector3 result = {};

        float l = inner_product(a, a);
        if (l > SQUARED_EPSILON) {
            result = a * inverse_sqrt(l);
        }

        return result;
    }

    // linear interpolate
    inline Vector3 lerp(Vector3 a, float t, Vector3 b)
    {
        Vector3 result = (1.0f - t) * a + t * b;
        return result;
    }

    inline Vector3 lerpVector3(Vector3 a, float t, Vector3 b)
    {
        Vector3 result = a * (1.0f - t) + b * t;
        return result;
    }

    float linear_to_sRGB(float l)
    {
        l = std::clamp(l, 0.0f, 1.0f);

        float s = l * SLOPE_HORIZONTAL;
        if (l > LINEAR_CUTOFF) {
            s = 1.055f * pow(l, 1.0f / 2.4f) - 0.055f;
        }

        return s;
    }

    inline uint32_t pack_BGRA(Vector3 unpacked)
    {
        uint32_t result = ((255 << 24) |
                           (round_float_to_uint32(unpacked.x) << 16) |
                           (round_float_to_uint32(unpacked.y) << 8) |
                           (round_float_to_uint32(unpacked.z) << 0));

        return result;
    }
}