/**
 * @file kmeans_multiplex.c
 * @author your name (you@domain.com)
 * @brief Запуск алгоритма k-средних в паралельном режиме
 * Паралелтзация через процессы
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <string.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
  // HACK: mmap и munmap стоит проверять на ошибку
  KMeans* tmp_kmeans =
      (KMeans*)mmap(NULL, sizeof(KMeans), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANON, -1, 0);
  tmp_kmeans->clusters_cnt = 3;
  tmp_kmeans->points_cnt = 12;
  // Кластеров не должно быть больше, чем самих точек
  if (tmp_kmeans->clusters_cnt > tmp_kmeans->points_cnt) {
    munmap(tmp_kmeans, sizeof(KMeans));
    return FAILURE;
  }
  tmp_kmeans->changed = tmp_kmeans->points_cnt;
  tmp_kmeans->points = (PointInCluster*)mmap(
      NULL, tmp_kmeans->points_cnt * sizeof(PointInCluster),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  tmp_kmeans->clusters =
      (Point*)mmap(NULL, tmp_kmeans->clusters_cnt * sizeof(Point),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  for (size_t i = 0; i < 4; i++) {
    tmp_kmeans->points[i].point.x = rand() % 50;
    tmp_kmeans->points[i].point.y = rand() % 50;
    tmp_kmeans->points[i].in_cluster = 0;

    tmp_kmeans->points[i + 4].point.x = 100 + rand() % 50;
    tmp_kmeans->points[i + 4].point.y = 100 + rand() % 50;
    tmp_kmeans->points[i + 4].in_cluster = 0;

    tmp_kmeans->points[i + 8].point.x = 1000 + rand() % 50;
    tmp_kmeans->points[i + 8].point.y = 1000 + rand() % 50;
    tmp_kmeans->points[i + 8].in_cluster = 0;
  }

  *kmeans = tmp_kmeans;
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
  // HACK: Стоит проверять munmap на ошибку
  KMeans* tmp_kmeans = *kmeans;
  if (tmp_kmeans->points != NULL) {
    munmap(tmp_kmeans->points, tmp_kmeans->points_cnt * sizeof(PointInCluster));
    tmp_kmeans->points = NULL;
  }
  if (tmp_kmeans->clusters != NULL) {
    munmap(tmp_kmeans->clusters, tmp_kmeans->clusters_cnt * sizeof(Point));
    tmp_kmeans->clusters = NULL;
  }
  munmap(tmp_kmeans, sizeof(KMeans));
  tmp_kmeans = NULL;

  *kmeans = tmp_kmeans;
  return SUCCESS;
}

#define MAX_SEND_SIZE 80
#define SORT_MSG 100
#define CENTER_MSG 200

typedef struct MsgBuf {
  long mtype;
  char mtext[MAX_SEND_SIZE];
} MsgBuf;

int SendMessage(int qid, long type, char* text) {
  static MsgBuf q_buf;
  q_buf.mtype = type;
  strcpy(q_buf.mtext, text);
  if (msgsnd(qid, &q_buf, strlen(q_buf.mtext) + 1, 0)) {
    return FAILURE;
  }
  return SUCCESS;
}

void ReadMessage(int qid, char* text, long type) {
  static MsgBuf q_buf;
  q_buf.mtype = type;
  msgrcv(qid, &q_buf, MAX_SEND_SIZE, type, 0);
  strcpy(text, q_buf.mtext);
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

  // TODO: Можно и больше процессов создать
  size_t process_cnt = kmeans->clusters_cnt;
  int pids[process_cnt];
  // HACK: Стоит проверить успех создания очереди
  int msdid = msgget(IPC_PRIVATE, IPC_CREAT | 0660);
  for (size_t i = 0; i < process_cnt; ++i) {
    pids[i] = fork();
    if (pids[i] == -1) {
      printf("fork failed\n");
      // HACK: Стоит убить уже созданные процессы
      return FAILURE;
    } else if (pids[i] == 0) {
      char recv_tmp[MAX_SEND_SIZE] = {0};
      // Фаза 1: сортируем по кластерам
      ReadMessage(msdid, recv_tmp, SORT_MSG);
      size_t start_batch = 0;
      size_t end_batch = 0;
      sscanf(recv_tmp, "%zu %zu", &start_batch, &end_batch);
      printf("Received %zu %zu\n", start_batch, end_batch);
      // TODO: Надо синхронизировать. (Возможно, ещё нужны прерывания).
      // Фаза 2: Поиск центров кластеров
      ReadMessage(msdid, recv_tmp, CENTER_MSG);
      size_t cnt = 0;
      sscanf(recv_tmp, "%zu", &cnt);
      printf("Received %zu\n", cnt);
      // TODO: Вырубаемся по команде родителя
      exit(0);
    }
  }

  char send_tmp[MAX_SEND_SIZE] = {0};
  // while (((float)kmeans->changed / (float)kmeans->points_cnt) > threshold) {
  // Отправляем сообщения по количеству процессов
  size_t start_batch = 0;
  size_t end_batch = 0;
  for (size_t i = 0; i < process_cnt; ++i) {
    start_batch = end_batch;
    // Таким подсчетом откусывается всегда "поровну" для всех оставшихся
    // процессов
    end_batch =
        start_batch + (kmeans->points_cnt - start_batch) / (process_cnt - i);
    snprintf(send_tmp, MAX_SEND_SIZE, "%zu %zu", start_batch, end_batch);
    SendMessage(msdid, SORT_MSG, send_tmp);
  }
  // TODO: Надо синхронизировать
  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    snprintf(send_tmp, MAX_SEND_SIZE, "%zu", i);
    SendMessage(msdid, CENTER_MSG, send_tmp);
  }
  // }

  for (size_t i = 0; i < process_cnt; ++i) {
    int status = 0;
    pid_t waited_pid = wait(&status);
    printf("Got waited_pid=%d, status %d\n", waited_pid, status);
  }

  return SUCCESS;
}

int PrintClusters(KMeans const* kmeans) {
  if (kmeans == NULL) {
    return FAILURE;
  }

  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    printf("num: %zu, x: %f, y: %f\n", i, kmeans->clusters[i].x,
           kmeans->clusters[i].y);
    for (size_t j = 0; j < kmeans->points_cnt; ++j) {
      if (kmeans->points[j].in_cluster == i) {
        printf("x: %f, y: %f\n", kmeans->points[j].point.x,
               kmeans->points[j].point.y);
      }
    }
  }
  return SUCCESS;
}

int main() {
  KMeans* kmeans = NULL;
  if (CreatPoints(&kmeans)) {
    printf("Bad 1");
  }
  if (StartAlgorithm(kmeans)) {
    printf("Bad 2");
  }

  if (PrintClusters(kmeans)) {
    printf("Bad 3");
  }

  if (DeletePoints(&kmeans)) {
    printf("Bad 4");
  }
  return 0;
}