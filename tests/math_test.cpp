//#include "../include/Math.h"
#include "gtest/gtest.h"
#include <cmath>

float square_root(float a)
{
    return sqrt(a);
}

TEST(SquareRootTest, PositiveNos)
{
EXPECT_EQ (3.0f, square_root(9.0f));
}

