/**
 * @file kmeans.c
 * @author your name (you@domain.com)
 * @brief Алгоритм кластеризации k-средних
 * 1. Получить массив точек arr и число кластеров k
 * 2. Взять k первых точек - центры кластеров
 * 3. Для каждой оставшейся точки:
 * * 3.1 Посчитать расстояние до каджого центра кластера
 * * 3.2 Отнести точку к тому кластеру, до которого ближе всего
 * 4. Посчитать центры тяжести новых кластеров
 * 5. Повторять с пункста 3 до тех пор, пока алгоритм не стабилизируется:
 * * 5.1 Проще всего смотрет по точка, которые изменили свой кластер -
 * * если их меньше какого-то процента то кластеры сформировались.
 * @version 0.1
 * @date 2022-03-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kmeans.h"

/**
 * @brief Квадрат евклидового растояния
 *
 * @param point_a
 * @param point_b
 * @param out - результат
 * @return int
 */
int SquareEuclideanDistance(Point const* point_a,
                            Point const* point_b,
                            float* out) {
  if (out == NULL) {
    return FAILURE;
  }

  *out = pow((point_a->x - point_b->x), 2) + pow((point_a->y - point_b->y), 2) +
         pow((point_a->z - point_b->z), 2);

  return SUCCESS;
}

/**
 * @brief Сортировка по кластерам
 * Считает расстояние для каждой точки до каждого центра кластера.
 * По наименьшему значению привязывает точку к кластеру.
 * Также считает количество точек, изменивших свой кластер.
 * @param kmeans
 * @param batch_start
 * @param batch_end
 * @return int
 */
int ClusterSort(KMeans* kmeans, size_t batch_start, size_t batch_end) {
  if ((kmeans == NULL) || (batch_end > kmeans->points_cnt) ||
      (batch_start > batch_end)) {
    return FAILURE;
  }

  kmeans->changed = 0;
  for (size_t i = batch_start; i < batch_end; ++i) {
    float dist_min = 0;
    size_t near_cluster = 0;
    float tmp = 0;
    // Поиск ближайшего кластера.
    // Инициализация поиска минимума
    if (SquareEuclideanDistance(&kmeans->points[i].point, &kmeans->clusters[0],
                                &tmp)) {
      return FAILURE;
    }
    dist_min = tmp;
    // Пробегаемся по расстоянию до каждого кластера
    for (size_t j = 1; j < kmeans->clusters_cnt; ++j) {
      if (SquareEuclideanDistance(&kmeans->points[i].point,
                                  &kmeans->clusters[j], &tmp)) {
        return FAILURE;
      }
      // И сохраняем наименьший
      if (tmp < dist_min) {
        dist_min = tmp;
        near_cluster = j;
      }
    }
    // Заносим номер кластера в точку
    if (kmeans->points[i].in_cluster != near_cluster) {
      ++kmeans->changed;
      kmeans->points[i].in_cluster = near_cluster;
    }
  }

  return SUCCESS;
}

/**
 * @brief Поиск центра кластера
 * Считает центр тяжести  кластера  cluster_num по среднему значению всех точек.
 * Результат заносится в kmeans->clusters[cluster_num]
 * @param kmeans
 * @param cluster_num - не больше, чем kmeans->clusters_cnt
 * @return int
 */
int FindClusterCenter(KMeans const* kmeans, size_t cluster_num) {
  if ((kmeans == NULL) || (cluster_num >= kmeans->clusters_cnt)) {
    return FAILURE;
  }

  size_t point_in_cluster_cnt = 0;
  Point sum = {0, 0, 0};
  for (size_t i = 0; i < kmeans->points_cnt; ++i) {
    if (kmeans->points[i].in_cluster == cluster_num) {
      sum.x += kmeans->points[i].point.x;
      sum.y += kmeans->points[i].point.y;
      sum.z += kmeans->points[i].point.z;
      ++point_in_cluster_cnt;
    }
  }
  if (point_in_cluster_cnt == 0) {
    kmeans->clusters[cluster_num].x = 0;
    kmeans->clusters[cluster_num].y = 0;
    kmeans->clusters[cluster_num].z = 0;
  } else {
    kmeans->clusters[cluster_num].x = sum.x / point_in_cluster_cnt;
    kmeans->clusters[cluster_num].y = sum.y / point_in_cluster_cnt;
    kmeans->clusters[cluster_num].z = sum.z / point_in_cluster_cnt;
  }

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
