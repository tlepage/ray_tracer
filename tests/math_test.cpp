#include "../include/Math.h"
#include "gtest/gtest.h"

TEST(SquareRootTest, PositiveNos)
{
    EXPECT_EQ (3.0f, Math::square_root(9.0f));
    EXPECT_EQ (9.0f, Math::square_root(81.0f));
}

