#include "../include/Math.h"
#include "gtest/gtest.h"

TEST(SquareRootTest, ValidateNumericalResult)
{
    EXPECT_EQ (3.0f, Math::square_root(9.0f));
    EXPECT_EQ (9.0f, Math::square_root(81.0f));
    EXPECT_NE (11.0f, Math::square_root(100.0f));
}

TEST(HadamardProductTest, ValidateVectorResult)
{
    Vector::Vector3 a {1.0f, 2.0f, 3.0f};
    Vector::Vector3 b {2.0f, 3.0f, 4.0f};

    Vector::Vector3 expected_result {2.0f, 6.0f, 12.0f};
    Vector::Vector3 actual_result = Math::hadamard_product(a, b);

    EXPECT_FLOAT_EQ(expected_result.x, actual_result.x);
    EXPECT_FLOAT_EQ(expected_result.y, actual_result.y);
    EXPECT_FLOAT_EQ(expected_result.z, actual_result.z);
}

TEST(InnerProductTest, ValidateAcuteVectorsProducePositiveResult)
{
    Vector::Vector3 a {1.0f, 2.0f, 3.0f};
    Vector::Vector3 b {2.0f, 3.0f, 4.0f};

    EXPECT_FLOAT_EQ(20.0f, Math::inner_product(a, b));
}

TEST(InnerProductTest, ValidateObtuseVectorsProduceNegativeResult)
{
    Vector::Vector3 a {-1.0f, -2.0f, -3.0f};
    Vector::Vector3 b {1.0f, 2.0f, 3.0f};

    EXPECT_FLOAT_EQ(-14.0f, Math::inner_product(a, b));
}

TEST(InnerProductTest, ValidateOrthogonalVectorsProduceZeroResult)
{
    Vector::Vector3 a {2.0f, 4.0f, 1.0f};
    Vector::Vector3 b {2.0f, 1.0f, -8.0f};

    EXPECT_FLOAT_EQ(0.0f, Math::inner_product(a, b));
}

TEST(CrossProductTest, ValidateVectorResult)
{
    Vector::Vector3 a {1.0f, 2.0f, 3.0f};
    Vector::Vector3 b {2.0f, 3.0f, 4.0f};

    Vector::Vector3 expected_result {-1.0f, 2.0f, -1.0f};
    Vector::Vector3 actual_result = Math::cross_product(a, b);

    EXPECT_FLOAT_EQ(expected_result.x, actual_result.x);
    EXPECT_FLOAT_EQ(expected_result.y, actual_result.y);
    EXPECT_FLOAT_EQ(expected_result.z, actual_result.z);
}

TEST(NormalizeOrZeroTest, ValidateZeroIsReturnedIfLessThanSquaredEpsilon) {
    Vector::Vector3 a{0.0000001f, 0.0000001f, 0.0000001f};
    Vector::Vector3 zero_result = Math::normalize_or_zero(a);

    EXPECT_FLOAT_EQ(zero_result.x, 0.0f);
    EXPECT_FLOAT_EQ(zero_result.y, 0.0f);
    EXPECT_FLOAT_EQ(zero_result.z, 0.0f);
}

TEST(NormalizeOrZeroTest, ValidateNormalizedVectorIfGreaterThanSquaredEpsilon)
{
    Vector::Vector3 a {1.0f, 2.0f, 3.0f};
    Vector::Vector3 normalized_result = Math::normalize_or_zero(a);

    EXPECT_FLOAT_EQ(normalized_result.x, 0.26721427f);
    EXPECT_FLOAT_EQ(normalized_result.y, 0.53442854f);
    EXPECT_FLOAT_EQ(normalized_result.z, 0.80164278f);
}

TEST(LerpTest, ValidateNumericalResult)
{
    Vector::Vector3 a {1.0f, 2.0f, 3.0f};
    Vector::Vector3 b {2.0f, 4.0f, 9.0f};
    float percentage_along_line = 0.5f;

    Vector::Vector3 result = Math::lerp(a, percentage_along_line, b);
    EXPECT_FLOAT_EQ(result.x, 1.5f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

// cutoff is referring to LINEAR_CUTOFF = 0.0031308f;
TEST(LinearTosRGBTest, ValidateNumericalResultForNumberBelowCutoff)
{
    float l = 0.0015674;
    EXPECT_FLOAT_EQ(0.020250807f, Math::linear_to_sRGB(l));
}

// cutoff is referring to LINEAR_CUTOFF = 0.0031308f;
TEST(LinearTosRGBTest, ValidateNumericalResultForNumberAboveCutoff)
{
    float l = 0.869324f;
    EXPECT_FLOAT_EQ(0.94020253f, Math::linear_to_sRGB(l));
}

TEST(PackBGRATest, ValidateNumericalResult)
{
    Vector::Vector3 a = {1.0f, 2.0f, 3.0f};
    EXPECT_EQ(4278256131, Math::pack_BGRA(a));
    //                  A        R        G        B
    // 4278256131 => 11111111 00000001 00000010 00000011 in binary
    // 255 << 24 (11111111) alpha
    // 1 << 16   (00000001) red
    // 2 << 8    (00000010) green
    // 3 << 0    (00000011) blue
}
