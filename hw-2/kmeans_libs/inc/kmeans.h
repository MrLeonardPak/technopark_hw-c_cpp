/**
 * @file kmeans.h
 * @author Leonard Pak (leopak2000@gmail.com)
 * @brief Общий header файл для однопроцессорной и многопроцессорной реализации
 * @version 0.1
 * @date 2022-04-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef HW2_KMEANS_H
#define HW2_KMEANS_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define FAILURE -1
#define SUCCESS 0

typedef struct Point {
  int x;
  int y;
  int z;
} Point;

typedef struct PointInCluster {
  Point point;
  size_t in_cluster;
} PointInCluster;

typedef struct KMeans {
  PointInCluster* points;
  size_t points_cnt;
  Point* clusters;
  size_t clusters_cnt;
} KMeans;
// Общая реализация
int SquareEuclideanDistance(Point const* point_a,
                            Point const* point_b,
                            int* out);
int ClusterSort(KMeans* kmeans,
                size_t batch_start,
                size_t batch_end,
                size_t* changed_test);
int FindClusterCenter(KMeans const* kmeans, size_t cluster_num);
int DeletePoints(KMeans** kmeans);
// Разные реализации
int CreatPoints(KMeans** kmeans, char const* file_name);
int StartAlgorithm(KMeans* kmeans);
int PrintClusters(KMeans const* kmeans);
#endif  // HW2_KMEANS_H