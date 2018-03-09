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

    inline Vector3 operator*(const float a, const Vector3 b)
    {
        Vector3 result = {};

        result.x = a * b.x;
        result.y = a * b.y;
        result.z = a * b.z;

        return result;
    }

    inline Vector3 operator*(const Vector3 b, const float a)
    {
        Vector3 result = a * b;
        return result;
    }

    inline Vector3 &operator*=(Vector3 &b, const float a)
    {
        b = a * b;
        return b;
    }

    inline Vector3 operator/(const Vector3 b, const float a)
    {
        Vector3 result = (1.0f / a) * b;
        return result;
    }

    inline Vector3 &operator/=(Vector3 &b, const float a)
    {
        b = b / a;
        return b;
    }

    inline Vector3 operator-(const Vector3 a)
    {
        Vector3 result = {};

        result.x = -a.x;
        result.y = -a.y;
        result.z = -a.z;

        return result;
    }

    inline Vector3 operator+(const Vector3 a, const Vector3 b)
    {
        Vector3 result = {};

        result.x = a.x + b.x;
        result.y = a.y + b.y;
        result.z = a.z + b.z;

        return result;
    }

    inline Vector3 &operator+=(Vector3 &a, const Vector3 b)
    {
        a = a + b;
        return a;
    }

    inline Vector3 operator-(const Vector3 a, const Vector3 b)
    {
        Vector3 result = {};

        result.x = a.x - b.x;
        result.y = a.y - b.y;
        result.z = a.z - b.z;

        return result;
    }

    inline Vector3 &operator-=(Vector3 &a, const Vector3 b)
    {
        a = a - b;
        return a;
    }

    inline float square_root(float a)
    {
        auto result = sqrt(a);
        return result;
    }

    inline uint32_t round_float_to_uint32(const float f)
    {
        auto result = (uint32_t)lround(f);
        return result;
    }

    struct RandomSeries
    {
        uint32_t state;
    };

    inline uint32_t xor_shift(RandomSeries *series)
    {
        // xorshift https://en.wikipedia.org/wiki/Xorshift
        // used in place of a Linear Congruential Generator or Permutative Congruential Generator
        // this replaces rand() due to performance reasons listed below:

        //stats before rand() improvement
        // time=46521ms
        // total bounces = 1849847016
        // performance = 0.000025ms/bounce

        // stats after rand() improvement
        // time=17347ms
        // total bounces = 1849736461
        // performance = 0.000009ms/bounce

        // at least 4 calls deep with no actual randomness, no inlining whatsoever
        //// rand() used thread local storage, hence all the calls
        // rand.cpp
        // sub     rsp, 38h
        // call    __acrt_getptd()
        //   --> call  internal_getptd_noexit()
        //        --> call _crt_scoped_get_last_error_reset::
        //        --> call internal_get_ptd_head()
        //        --> call _crt_hmodule_traits::get_invalid_value()
        //            --> call qword ptr [__imp_GetLastError()]
        //            --> call qword ptr [__imp_SetLastError()]
        // mov     qword ptr [ptd], rax
        // mov     rax,qword ptr [ptd]
        // imul    eax,dword ptr [rax+28h], 343FDh
        // add     eax,269Ech3
        // mov     rcx,qword ptr [ptd]
        // mov     dword ptr [rcx+28h],eax
        // mov     rax,qword ptr [ptd]
        // mov     eax,dword ptr [rax+28h]
        // shr     eax,10h
        // and     eax,7FFFh
        // add     rsp,38h
        // ret
        uint32_t x = series->state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        series->state = x;
        return x;
    }

    float random_unilateral(RandomSeries *series)
    {
        float result = static_cast<float>(xor_shift(series)) / static_cast<float>(UINT32_MAX);
        return result;
    }

    float random_bilateral(RandomSeries *series)
    {
        float result = -1.0f + 2.0f * random_unilateral(series);
        return result;
    }

    inline Vector3 hadamard_product(const Vector3 a, const Vector3 b)
    {
        Vector3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
        return result;
    }

    inline float inner_product(const Vector3 a, const Vector3 b)
    {
        float result = a.x * b.x + a.y * b.y + a.z * b.z;
        return result;
    }

    inline Vector3 cross_product(const Vector3 a, const Vector3 b)
    {
        Vector3 result = {};

        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;

        return result;
    }

    // commonly used replacement for inverse square root due to performance
    // improvements over the standard call to sqrt
    // https://en.wikipedia.org/wiki/Fast_inverse_square_root
    float inverse_sqrt(const float x)
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
    inline Vector3 normalize_or_zero(const Vector3 a)
    {
        Vector3 result = {};

        float l = inner_product(a, a);
        if (l > SQUARED_EPSILON)
        {
            result = a * inverse_sqrt(l);
        }

        return result;
    }

    // linear interpolate
    inline Vector3 lerp(const Vector3 a, const float t, const Vector3 b)
    {
        Vector3 result = (1.0f - t) * a + t * b;
        return result;
    }

    float linear_to_sRGB(float l)
    {
        l = std::clamp(l, 0.0f, 1.0f);

        float s = l * SLOPE_HORIZONTAL;
        if (l > LINEAR_CUTOFF)
        {
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
} // namespace Math
