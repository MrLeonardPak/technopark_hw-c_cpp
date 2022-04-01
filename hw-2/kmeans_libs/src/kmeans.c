/**
 * @file kmeans.c
 * @author Leonard Pak (leopak2000@gmail.com)
 * @brief Определение функций с одинаковой реализаций для однопроцессорного и
 * многопроцессорного варианта.
 * Алгоритм кластеризации k-средних:
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
 * @param out
 * @return int
 */
int SquareEuclideanDistance(Point const* point_a,
                            Point const* point_b,
                            unsigned long* out) {
  if ((out == NULL) || (point_a == NULL) || (point_b == NULL)) {
    return FAILURE;
  }
  // Координаты точек - это int. В худшем случае имеем delta от -INT_MAX ДО
  // INT_MAX, которое затем надо возвести во 2 степень. Тогда надо иметь
  // небольшой запас по значениям + ответ всё равно положительный
  // Возможно, лучше использовать расстояние Чебышева
  unsigned long delta_x = (unsigned long)abs(point_a->x - point_b->x);
  unsigned long delta_y = (unsigned long)abs(point_a->y - point_b->y);
  unsigned long delta_z = (unsigned long)abs(point_a->z - point_b->z);
  *out = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;

  return SUCCESS;
}

/**
 * @brief Сортировка по кластерам.
 * Считает расстояние для каждой точки до каждого центра кластера.
 * По наименьшему значению привязывает точку к кластеру.
 * Также считает количество точек, изменивших свой кластер.
 * @param kmeans
 * @param batch_start
 * @param batch_end
 * @param changed
 * @return int
 */
int ClusterSort(KMeans* kmeans,
                size_t batch_start,
                size_t batch_end,
                size_t* changed) {
  if ((kmeans == NULL) || (batch_end > kmeans->points_cnt) ||
      (batch_start > batch_end) || (changed == NULL) ||
      (kmeans->points == NULL) || (kmeans->clusters == NULL)) {
    return FAILURE;
  }

  *changed = 0;
  for (size_t i = batch_start; i < batch_end; ++i) {
    unsigned long dist_min = ULONG_MAX;
    size_t near_cluster = 0;
    unsigned long tmp = 0;
    // Поиск ближайшего кластера. По аналогии обычного поиска минимума
    for (size_t j = 0; j < kmeans->clusters_cnt; ++j) {
      // Тут проверка возврата лишнее, тк всё необходимое же проверено
      SquareEuclideanDistance(&kmeans->points[i].point, &kmeans->clusters[j],
                              &tmp);
      // И сохраняем наименьший
      if (tmp < dist_min) {
        dist_min = tmp;
        near_cluster = j;
      }
    }
    // Заносим номер кластера в точку
    if (kmeans->points[i].in_cluster != near_cluster) {
      kmeans->points[i].in_cluster = near_cluster;
      ++(*changed);
    }
  }

  return SUCCESS;
}

/**
 * @brief Поиск центра кластера.
 * Считает центр тяжести кластера cluster_num по среднему значению координат
 * всех точек. Результат заносится в kmeans->clusters[cluster_num]
 * @param kmeans
 * @param cluster_num
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
  if (point_in_cluster_cnt > 0) {
    kmeans->clusters[cluster_num].x = sum.x / point_in_cluster_cnt;
    kmeans->clusters[cluster_num].y = sum.y / point_in_cluster_cnt;
    kmeans->clusters[cluster_num].z = sum.z / point_in_cluster_cnt;
  }

  return SUCCESS;
}

int PrintClusters(KMeans const* kmeans) {
  if (kmeans == NULL) {
    return FAILURE;
  }

  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    printf("num: %zu, x: %d, y: %d\n", i, kmeans->clusters[i].x,
           kmeans->clusters[i].y);
    for (size_t j = 0; j < kmeans->points_cnt; ++j) {
      if (kmeans->points[j].in_cluster == i) {
        printf("x: %d, y: %d\n", kmeans->points[j].point.x,
               kmeans->points[j].point.y);
      }
    }
  }
  return SUCCESS;
}
