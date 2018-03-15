#pragma once
#include <cmath>
#include <algorithm>
#include "Vector.h"

constexpr uint32_t LOMONT_CONSTANT = 0x5f375a86;
constexpr float SQUARED_EPSILON = 0.00000001f;
constexpr float LINEAR_CUTOFF = 0.0031308f;
constexpr float SLOPE_HORIZONTAL = 12.92f;

namespace Math
{
    inline auto square_root(float a)
    {
        return sqrt(a);
    }

    inline auto round_float_to_uint32(float f)
    {
        return (uint32_t)lround(f);
    }

    struct RandomSeries
    {
        uint32_t state;
    };

    inline auto xor_shift(RandomSeries *series)
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

    inline auto random_unilateral(RandomSeries *series)
    {
        return static_cast<float>(xor_shift(series)) / static_cast<float>(UINT32_MAX);
    }

    inline auto random_bilateral(RandomSeries *series)
    {
        return (-1.0f + 2.0f * random_unilateral(series));
    }

    inline auto hadamard_product(const Vector::Vector3 &a, const Vector::Vector3 &b)
    {
        Vector::Vector3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
        return result;
    }

    inline auto inner_product(const Vector::Vector3 &a, const Vector::Vector3 &b)
    {
        float result = a.x * b.x + a.y * b.y + a.z * b.z;
        return result;
    }

    inline auto cross_product(const Vector::Vector3 &a, const Vector::Vector3 &b)
    {
        Vector::Vector3 result = {};

        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;

        return result;
    }

    // commonly used replacement for inverse square root due to performance
    // improvements over the standard call to sqrt
    // https://en.wikipedia.org/wiki/Fast_inverse_square_root
    inline auto inverse_sqrt(const float x)
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
    inline auto normalize_or_zero(const Vector::Vector3 &a)
    {
        Vector::Vector3 result = {};

        float l = inner_product(a, a);
        if (l > SQUARED_EPSILON)
        {
            result = a * inverse_sqrt(l);
        }

        return result;
    }

    // linear interpolate
    inline auto lerp(const Vector::Vector3 &a, const float t, const Vector::Vector3 &b)
    {
        Vector::Vector3 result = (1.0f - t) * a + t * b;
        return result;
    }

    inline auto linear_to_sRGB(float l)
    {
        l = std::clamp(l, 0.0f, 1.0f);

        float s = l * SLOPE_HORIZONTAL;
        if (l > LINEAR_CUTOFF)
        {
            s = 1.055f * pow(l, 1.0f / 2.4f) - 0.055f;
        }

        return s;
    }

    inline auto pack_BGRA(Vector::Vector3 &unpacked)
    {
        uint32_t result = ((255 << 24) |
                           (round_float_to_uint32(unpacked.x) << 16) |
                           (round_float_to_uint32(unpacked.y) << 8) |
                           (round_float_to_uint32(unpacked.z) << 0));

        return result;
    }
} // namespace Math
