/**
 * lcov -t "tests/tests_kmeans" -o coverage.info -c -d kmeans_libs/
 * genhtml -o report coverage.info
 * cd report && python3 -m http.server 8000
 */
#include "gtest/gtest.h"

extern "C" {
#include "kmeans.h"
}

TEST(SERIAL_TESTS, CreatPoints_TEST) {
  KMeans* kmeans = NULL;
  int status = system("data/data 2 /tmp/data_false.bin");
  ASSERT_EQ(status, 0);
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, CreatPoints(NULL, "/tmp/data_false.bin"));
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, NULL));
  // Проверка на неправильный файл
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, "no_file.bin"));
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, ""));
  // Проверка на неправильные данные
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, "/tmp/data_false.bin"));
}

TEST(SERIAL_TESTS, StartAlgorithm_TEST) {
  KMeans* kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  kmeans->points_cnt = 3;
  kmeans->clusters_cnt = 2;
  // Проверка на передачу неправилных данных
  EXPECT_EQ(FAILURE, StartAlgorithm(NULL));
  kmeans->points_cnt = 0;
  EXPECT_EQ(FAILURE, StartAlgorithm(kmeans));
  kmeans->points_cnt = 1;
  kmeans->clusters_cnt = 0;
  EXPECT_EQ(FAILURE, StartAlgorithm(kmeans));
  kmeans->clusters_cnt = 2;
  EXPECT_EQ(FAILURE, StartAlgorithm(kmeans));
  // Освобождение памяти
  free(kmeans);
}

TEST(SERIAL_TESTS, DeletePoints_TEST) {
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, DeletePoints(NULL));
}

TEST(SERIAL_TESTS, MAIN_TEST) {
  KMeans* kmeans = NULL;
  int status = system("data/data 1 /tmp/data.bin");
  ASSERT_EQ(status, 0);
  // Проверка на полную работу
  ASSERT_EQ(SUCCESS, CreatPoints(&kmeans, "/tmp/data.bin"));
  EXPECT_EQ(SUCCESS, StartAlgorithm(kmeans));
  EXPECT_EQ(SUCCESS, PrintClusters(kmeans));
  EXPECT_EQ(SUCCESS, DeletePoints(&kmeans));
}