/**
 * @file test.cc
 * @author your name (you@domain.com)
 * @brief
 * lcov -t "tests/tests_kmeans" -o coverage.info -c -d kmeans_libs/
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
  size_t const ans_cnt = 3;
  Point const points_a[ans_cnt] = {{0, 0, 0}, {1, 2, 3}, {2, 2, 2}};
  Point const points_b[ans_cnt] = {{0, 0, 0}, {3, 2, 1}, {-2, -2, -2}};
  unsigned long const right_answers[ans_cnt] = {0, 8, 48};
  unsigned long answer = 0;
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, SquareEuclideanDistance(NULL, &points_b[0], &answer));
  EXPECT_EQ(FAILURE, SquareEuclideanDistance(&points_a[0], NULL, &answer));
  EXPECT_EQ(FAILURE, SquareEuclideanDistance(&points_a[0], &points_b[0], NULL));
  //Проверка на правильный вывод
  for (size_t i = 0; i < ans_cnt; ++i) {
    EXPECT_EQ(SUCCESS,
              SquareEuclideanDistance(&points_a[i], &points_b[i], &answer));
    EXPECT_EQ(right_answers[i], answer);
  }
}

TEST(KMEANS_TESTS, FindClusterCentere_TEST) {
  KMeans* kmeans = (KMeans*)calloc(1, sizeof(KMeans));
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, FindClusterCenter(NULL, 0));
  EXPECT_EQ(FAILURE, FindClusterCenter(kmeans, kmeans->clusters_cnt + 1));
  //Проверка на правильный вывод
  kmeans->clusters_cnt = 1;
  kmeans->clusters = (Point*)malloc(kmeans->clusters_cnt * sizeof(Point));
  kmeans->points_cnt = 4;
  kmeans->points =
      (PointInCluster*)malloc(sizeof(PointInCluster) * kmeans->points_cnt);
  Point const right_answers = {2, 2, 2};
  kmeans->points[0] = {1, 1, 1, 0};
  kmeans->points[1] = {3, 3, 3, 0};
  kmeans->points[2] = {1, 3, 1, 0};
  kmeans->points[3] = {3, 1, 3, 0};
  EXPECT_EQ(SUCCESS, FindClusterCenter(kmeans, 0));
  EXPECT_EQ(right_answers.x, kmeans->clusters[0].x);
  EXPECT_EQ(right_answers.y, kmeans->clusters[0].y);
  EXPECT_EQ(right_answers.z, kmeans->clusters[0].z);

  free(kmeans->clusters);
  free(kmeans->points);
  free(kmeans);
}

TEST(KMEANS_TESTS, ClusterSort_TEST) {
  KMeans* kmeans = (KMeans*)calloc(1, sizeof(KMeans));
  kmeans->points_cnt = 3;
  size_t changed = 0;
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, ClusterSort(NULL, 0, 1, &changed));
  EXPECT_EQ(FAILURE, ClusterSort(kmeans, 0, 1, NULL));
  EXPECT_EQ(FAILURE, ClusterSort(kmeans, 0, kmeans->points_cnt + 1, &changed));
  EXPECT_EQ(FAILURE, ClusterSort(kmeans, 1, 0, &changed));

  free(kmeans);
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

  ASSERT_EQ(SUCCESS, CreatPoints(&kmeans, "/tmp/data.bin"));
  EXPECT_EQ(SUCCESS, StartAlgorithm(kmeans));
  EXPECT_EQ(SUCCESS, PrintClusters(kmeans));
  EXPECT_EQ(SUCCESS, DeletePoints(&kmeans));
}