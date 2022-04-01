/**
 * @file kmeans_serial.c
 * @author Leonard Pak (leopak2000@gmail.com)
 * @brief Запуск алгоритма k-средних в последовательном режиме.
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kmeans.h"

const float threshold = 0.01;

/**
 * @brief Создает специальную структуру из бинарного файла
 *
 * @param kmeans
 * @param file_name
 * @return int
 */
int CreatPoints(KMeans** kmeans, char const* file_name) {
  if ((kmeans == NULL) || (*kmeans != NULL)) {  // file_name проверит fopen()
    return FAILURE;
  }

  FILE* fptr = NULL;
  fptr = fopen(file_name, "rb");
  if (fptr == NULL) {
    return FAILURE;
  }
  KMeans* tmp_kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  // В начале файла расположены количество точек и необходимое число кластеров
  fread(&tmp_kmeans->points_cnt, sizeof(size_t), 1, fptr);
  fread(&tmp_kmeans->clusters_cnt, sizeof(size_t), 1, fptr);
  // Кластеров не должно быть больше, чем самих точек
  if (tmp_kmeans->clusters_cnt > tmp_kmeans->points_cnt) {
    free(tmp_kmeans);
    fclose(fptr);
    return FAILURE;
  }
  tmp_kmeans->points =
      (PointInCluster*)malloc(tmp_kmeans->points_cnt * sizeof(PointInCluster));
  tmp_kmeans->clusters =
      (Point*)malloc(tmp_kmeans->clusters_cnt * sizeof(Point));
  // Далее расположены сами точки
  for (size_t i = 0; i < tmp_kmeans->points_cnt; ++i) {
    fread(&tmp_kmeans->points[i].point, sizeof(Point), 1, fptr);
    tmp_kmeans->points[i].in_cluster = 0;
  }

  *kmeans = tmp_kmeans;
  fclose(fptr);
  return SUCCESS;
}

/**
 * @brief Запуск алгоритма
 *
 * @param kmeans
 * @return int
 */
int StartAlgorithm(KMeans* kmeans) {
  if ((kmeans == NULL) || (kmeans->points_cnt == 0) ||
      (kmeans->clusters_cnt == 0) ||
      (kmeans->clusters_cnt > kmeans->points_cnt)) {
    return FAILURE;
  }
  // За первые центры кластеров берутся первые точки из данных
  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    kmeans->clusters[i] = kmeans->points[i].point;
  }
  // Условие выхода из цикла - алгоритм сошелся в какой-то диапазон
  size_t changed = 0;
  do {
    // Тут проверка возврата лишнее, тк всё необходимое же проверено
    ClusterSort(kmeans, 0, kmeans->points_cnt, &changed);
    for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
      // Тут проверка возврата лишнее, тк всё необходимое же проверено
      FindClusterCenter(kmeans, i);
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
