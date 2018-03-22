#pragma once
#include <cmath>
#include <algorithm>
#include "Vector.h"

namespace Math
{
    constexpr uint32_t LOMONT_CONSTANT = 0x5f375a86;
    constexpr float SQUARED_EPSILON = 0.00000001f;
    constexpr float LINEAR_CUTOFF = 0.0031308f;
    constexpr float SLOPE_HORIZONTAL = 12.92f;

    struct RandomSeries
    {
        uint32_t state;
    };

    // The newer Mersenne Twister algorithm in the C++ LCG library can provide results
    // that are close to xor_shift; however, those results may vary depending on which
    // compiler you are using.  In some instances (clang), those results can be *slower* than
    // std::rand().  For this project, I chose to use xor_shift since it is very fast
    // and accomplishes what I need for randomness
    // https://medium.com/@odarbelaeze/how-competitive-are-c-standard-random-number-generators-f3de98d973f0
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
        // rand() used thread local storage, hence all the calls
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

    inline auto square_root(float a)
    {
        return sqrt(a);
    }

    // https://en.wikipedia.org/wiki/Hadamard_product_(matrices)
    // The Hadamard Product is an operation that takes two matrices of the same
    // dimension and produces a resultant matrix of the same dimension; not the same
    // as a matrix product operation
    inline auto hadamard_product(const Vector::Vector3 &a, const Vector::Vector3 &b)
    {
        return Vector::Vector3 {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    // https://en.wikipedia.org/wiki/Dot_product
    // scalar function of two vectors equal to the product of their magnitudes
    // and the cosine of the angle between them, also called dot product
    // can be used to determine if vectors are obtuse(negative values),
    // acute(positive values) or are orthogonal(0.0f) based on the result of the product
    inline auto inner_product(const Vector::Vector3 &a, const Vector::Vector3 &b)
    {
        return (a.x * b.x + a.y * b.y + a.z * b.z);
    }

    // https://en.wikipedia.org/wiki/Cross_product
    // Vector operation where the resultant vector is perpendicular to both input vectors
    inline auto cross_product(const Vector::Vector3 &a, const Vector::Vector3 &b)
    {
        return Vector::Vector3
        {
             a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x
        };
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
    // get length, if length is greater than epsilon, normalize the vector;
    // otherwise return zero
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

    // https://en.wikipedia.org/wiki/Linear_interpolation
    // This function is used to find a value that is some percentage between two
    // known values.  In this case, using a linear polynomial.  This is often called
    // "lerp" in computer graphics
    inline auto lerp(const Vector::Vector3 &a, const float t, const Vector::Vector3 &b)
    {
        return Vector::Vector3 {(1.0f - t) * a + t * b};
    }

    // http://entropymine.com/imageworsener/srgbformula/
    // https://en.wikipedia.org/wiki/SRGB
    // This function converts linear space color values to sRGB color space values
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

    // takes a vector representation of the color (unpacked) and packs it
    // into a binary format for each pixel
    // packs alpha in the highest bits, then red, green, and blue
    inline auto pack_BGRA(Vector::Vector3 &unpacked)
    {
        uint32_t result = ((255 << 24) |
                           (static_cast<uint32_t>(lround(unpacked.x)) << 16) |
                           (static_cast<uint32_t>(lround(unpacked.y)) << 8)  |
                           (static_cast<uint32_t>(lround(unpacked.z)) << 0));

        return result;
    }
} // namespace Math
