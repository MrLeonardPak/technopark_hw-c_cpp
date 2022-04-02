/**
 * @file main.c
 * @author Леонард Пак
 * @brief Вариант #13
В вашем распоряжении — массив из 33 млн. трехмерных радиус-векторов.
Реализуйте алгоритм кластеризации KMeans, а затем распараллельте его с
использованием нескольких процессов с учетом оптимизации работы с кэш-памятью.
 * @version 0.1
 * @date 2022-03-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kmeans.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    return -1;
  }

  KMeans* kmeans = NULL;
  char const* file_name = argv[1];
  if (CreatPoints(&kmeans, file_name)) {
    return -1;
  }
  if (StartAlgorithm(kmeans)) {
    if (DeletePoints(&kmeans)) {
      return -1;
    }
  }
  WriteClusters(kmeans, argv[2]);
  if (DeletePoints(&kmeans)) {
    return -1;
  }
  return 0;
}