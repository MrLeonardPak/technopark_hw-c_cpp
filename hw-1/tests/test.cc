#include "gtest/gtest.h"

extern "C" {
#include "schedule.h"
}

TEST(Tests, TestBasics) {
  EXPECT_EQ(1, 1);
  EXPECT_EQ(1, 0);
  EXPECT_EQ(1, 1);
}