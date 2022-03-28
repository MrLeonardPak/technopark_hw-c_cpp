#include <math.h>
#include <stddef.h>

#define FAILURE -1
#define SUCCESS 0

typedef struct Point {
  float x;
  float y;
  float z;
} Point;

typedef struct PointInCluster {
  Point point;
  int in_cluster;
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
int ClusterSort(KMeans* kmeans);
int FindClusterCenter(KMeans const* kmeans, size_t cluster_num);