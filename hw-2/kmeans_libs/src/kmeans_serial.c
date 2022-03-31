/**
 * @file kmeans_serial.c
 * @author your name (you@domain.com)
 * @brief Запуск алгоритма k-средних в последовательном режиме
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kmeans.h"

const float threshold = 0.01;

/**
 * @brief Создает специальную структуру из TODO:
 *
 * @param kmeans - должен быть пустым и NULL
 * @return int
 */
int CreatPoints(KMeans** kmeans) {
  if (*kmeans != NULL) {
    return FAILURE;
  }
  // TODO: Переписать под прием из файла
  KMeans* tmp_kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  tmp_kmeans->clusters_cnt = 3;
  tmp_kmeans->points_cnt = 12;
  // Кластеров не должно быть больше, чем самих точек
  if (tmp_kmeans->clusters_cnt > tmp_kmeans->points_cnt) {
    free(tmp_kmeans);
    return FAILURE;
  }
  tmp_kmeans->points =
      (PointInCluster*)malloc(tmp_kmeans->points_cnt * sizeof(PointInCluster));
  tmp_kmeans->clusters =
      (Point*)malloc(tmp_kmeans->clusters_cnt * sizeof(Point));
  for (size_t i = 0; i < 4; i++) {
    tmp_kmeans->points[i].point.x = rand() % 50;
    tmp_kmeans->points[i].point.y = rand() % 50;
    tmp_kmeans->points[i].point.z = rand() % 50;
    tmp_kmeans->points[i].in_cluster = 0;

    tmp_kmeans->points[i + 4].point.x = 100 + rand() % 50;
    tmp_kmeans->points[i + 4].point.y = 100 + rand() % 50;
    tmp_kmeans->points[i + 4].point.z = 100 + rand() % 50;
    tmp_kmeans->points[i + 4].in_cluster = 0;

    tmp_kmeans->points[i + 8].point.x = 1000 + rand() % 50;
    tmp_kmeans->points[i + 8].point.y = 1000 + rand() % 50;
    tmp_kmeans->points[i + 8].point.z = 1000 + rand() % 50;
    tmp_kmeans->points[i + 8].in_cluster = 0;
  }

  *kmeans = tmp_kmeans;
  return SUCCESS;
}

/**
 * @brief Запуск алгоритма
 *
 * @param kmeans -необходимо заранее создать через вызов CreatPoints()
 * @return int
 */
int StartAlgorithm(KMeans* kmeans) {
  // За первые центры кластеров берутся первые точки из данных
  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    kmeans->clusters[i] = kmeans->points[i].point;
  }
  // HACK: (Можно убрать) Нельзя, чтобы первые точки были одинаковые
  for (size_t i = 0, k = 0; i < kmeans->clusters_cnt; ++i) {
    for (size_t j = i + 1; j < kmeans->clusters_cnt; j++) {
      if ((kmeans->clusters[j].x == kmeans->clusters[i].x) &&
          (kmeans->clusters[j].y == kmeans->clusters[i].y) &&
          (kmeans->clusters[j].z == kmeans->clusters[i].z)) {
        // Проверяем, не дошли ли до конца массива доступных точек
        if (kmeans->clusters_cnt + k >= kmeans->points_cnt) {
          return FAILURE;
        }
        kmeans->clusters[i] = kmeans->points[kmeans->clusters_cnt + k++].point;
        i = 0;
        break;
      }
    }
  }
  size_t changed = 0;
  do {
    if (ClusterSort(kmeans, 0, kmeans->points_cnt, &changed)) {
      return FAILURE;
    }
    for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
      if (FindClusterCenter(kmeans, i)) {
        return FAILURE;
      }
    }
  } while (((float)changed / (float)kmeans->points_cnt) > threshold);

  return SUCCESS;
}

/**
 * @brief Аккуратное удаление выделенной памяти
 *
 * @param kmeans
 * @return int
 */
int DeletePoints(KMeans** kmeans) {
  if ((kmeans == NULL) || (*kmeans == NULL)) {
    return FAILURE;
  }

  KMeans* tmp_kmeans = *kmeans;
  if (tmp_kmeans->points != NULL) {
    free(tmp_kmeans->points);
    tmp_kmeans->points = NULL;
  }
  if (tmp_kmeans->clusters != NULL) {
    free(tmp_kmeans->clusters);
    tmp_kmeans->clusters = NULL;
  }
  free(tmp_kmeans);
  tmp_kmeans = NULL;

  *kmeans = tmp_kmeans;
  return SUCCESS;
}
