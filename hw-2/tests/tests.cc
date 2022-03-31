/**
 * @file test.cc
 * @author your name (you@domain.com)
 * @brief
 * lcov -t "tests/tests_schedule" -o coverage.info -c -d schedule/
 * genhtml -o report coverage.info
 * cd report && python3 -m http.server 8000
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "gtest/gtest.h"

extern "C" {
#include "kmeans.h"
}

TEST(KMEANS_TESTS, SquareEuclideanDistance_TEST) {
  Point points_a[] = {{0, 0, 0}, {1, 2, 3}};
  Point points_b[] = {{0, 0, 0}, {3, 2, 1}};
  int right_answers[] = {0, 8};
  int answer = 0;

  EXPECT_EQ(SUCCESS,
            SquareEuclideanDistance(&points_a[0], &points_b[0], &answer));
  EXPECT_EQ(right_answers[0], answer);
  EXPECT_EQ(SUCCESS,
            SquareEuclideanDistance(&points_a[1], &points_b[1], &answer));
  EXPECT_EQ(right_answers[1], answer);

  EXPECT_EQ(FAILURE, SquareEuclideanDistance(NULL, &points_b[0], &answer));
  EXPECT_EQ(FAILURE, SquareEuclideanDistance(&points_a[0], NULL, &answer));
  EXPECT_EQ(FAILURE, SquareEuclideanDistance(&points_a[0], &points_b[0], NULL));
}