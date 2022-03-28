#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define FAILURE -1
#define SUCCESS 0

typedef struct Point {
  float x;
  float y;
  float z;
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
  size_t changed;
} KMeans;

int SquareEuclideanDistance(Point const* point_a,
                            Point const* point_b,
                            float* out);
int ClusterSort(KMeans* kmeans, size_t batch_start, size_t batch_end);
int FindClusterCenter(KMeans const* kmeans, size_t cluster_num);
int DeletePoints(KMeans** kmeans);

int CreatPoints(KMeans** kmeans);
int StartAlgorithm(KMeans* kmeans);
void PrintClusters(KMeans const* kmeans);
