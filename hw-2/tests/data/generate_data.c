/**
 * @file generate_data.c
 * @author your name (you@domain.com)
 * @brief Генерация точек для калстеризации
 * @version 0.1
 * @date 2022-04-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FAILURE -1
#define SUCCESS 0

typedef struct Point {
  int x;
  int y;
  int z;
} Point;

static int CreatData(char const* file_name,
                     size_t points_cnt,
                     size_t clusters_cnt,
                     int shift,
                     int dispersion);
static int WritePoint(FILE* fptr, int lower_limit, int upper_limit);
static int CreatFasleData(char const* file_name, int error_type);

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Invalid input arguments!\n");
    return FAILURE;
  }
  int type = atoi(argv[1]);
  int rtn = 0;
  switch (type) {
    case 0:
      rtn = CreatData(argv[2], 33000000, 10, 1000000, 500000);
      break;
    case 1:
      rtn = CreatData(argv[2], 12, 3, 2000, 1000);
      break;
    case 2:
      rtn = CreatFasleData(argv[2], type);
      break;
    default:
      rtn = FAILURE;
      break;
  }
  return rtn;
}

static int CreatData(char const* file_name,
                     size_t points_cnt,
                     size_t clusters_cnt,
                     int shift,
                     int dispersion) {
  FILE* fptr;
  fptr = fopen(file_name, "wb");
  if (fptr == NULL) {
    return FAILURE;
  }

  if (fwrite(&points_cnt, sizeof(size_t), 1, fptr) != 1) {
    fclose(fptr);
    return FAILURE;
  }
  if (fwrite(&clusters_cnt, sizeof(size_t), 1, fptr) != 1) {
    fclose(fptr);
    return FAILURE;
  }

  size_t start_cluster = 0;
  size_t end_cluster = 0;
  for (size_t i = 0; i < clusters_cnt; ++i) {
    start_cluster = end_cluster;
    // Таким подсчетом откусывается всегда "поровну" для всех оставшихся
    // процессов
    end_cluster =
        start_cluster + (points_cnt - start_cluster) / (clusters_cnt - i);
    for (size_t j = start_cluster; j < end_cluster; ++j) {
      if (WritePoint(fptr, i * shift, dispersion)) {
        fclose(fptr);
        return FAILURE;
      }
    }
  }

  char eof = '\0';
  if (fputc(eof, fptr) != eof) {
    fclose(fptr);
    return FAILURE;
  }
  if (fclose(fptr)) {
    return FAILURE;
  }
  return SUCCESS;
}

static int WritePoint(FILE* fptr, int lower_limit, int dispersion) {
  if ((fptr == NULL)) {
    return FAILURE;
  }
  Point point;
  // unsigned int seed = time(NULL);

  point.x = rand() % dispersion + lower_limit;
  point.y = rand() % dispersion + lower_limit;
  point.z = rand() % dispersion + lower_limit;

  if (fwrite(&point, sizeof(Point), 1, fptr) != 1) {
    return FAILURE;
  }
  return SUCCESS;
}

static int CreatFasleData(char const* file_name, int error_type) {
  FILE* fptr;
  switch (error_type) {
    case 2:
      // Количество кластеров больше, чем количество точек
      fptr = fopen(file_name, "wb");
      if (fptr == NULL) {
        return FAILURE;
      }
      size_t points_cnt = 5;
      size_t clusters_cnt = 10;
      if (fwrite(&points_cnt, sizeof(size_t), 1, fptr) != 1) {
        fclose(fptr);
        return FAILURE;
      }
      if (fwrite(&clusters_cnt, sizeof(size_t), 1, fptr) != 1) {
        fclose(fptr);
        return FAILURE;
      }
      return SUCCESS;
      break;

    default:
      return FAILURE;
      break;
  }
}