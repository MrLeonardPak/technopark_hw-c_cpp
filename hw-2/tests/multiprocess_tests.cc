/**
 * lcov -t "tests/tests_kmeans" -o coverage.info -c -d kmeans_libs/
 * genhtml -o report coverage.info
 * cd report && python3 -m http.server 8000
 */
#include "gtest/gtest.h"

extern "C" {
#include "kmeans.h"
}

TEST(KMEANS_TESTS, MAIN_TEST) {
  KMeans* kmeans = NULL;
  int status = system("data/data 1 /tmp/data.bin");
  ASSERT_EQ(status, 0);
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, CreatPoints(NULL, "/tmp/data.bin"));
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, NULL));
  EXPECT_EQ(FAILURE, StartAlgorithm(NULL));
  EXPECT_EQ(FAILURE, PrintClusters(NULL));
  EXPECT_EQ(FAILURE, DeletePoints(NULL));
  // Проверка на полную работу
  ASSERT_EQ(SUCCESS, CreatPoints(&kmeans, "/tmp/data.bin"));
  EXPECT_EQ(SUCCESS, StartAlgorithm(kmeans));
  EXPECT_EQ(SUCCESS, PrintClusters(kmeans));
  EXPECT_EQ(SUCCESS, DeletePoints(&kmeans));
}