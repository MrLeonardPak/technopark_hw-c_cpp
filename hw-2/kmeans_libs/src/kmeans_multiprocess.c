/**
 * @file kmeans_multiprocess.c
 * @author Leonard Pak (leopak2000@gmail.com)
 * @brief Запуск алгоритма k-средних в паралельном режиме (процессы)
 *
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kmeans.h"

#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_SEND_SIZE 80

#define SORT_MSG 100
#define CENTER_MSG 200
#define TO_PARENT_MSG 300

typedef struct MsgBuf {
  long mtype;
  char mtext[MAX_SEND_SIZE];
} MsgBuf;

const float threshold = 0.1;
static int phase_num = 0;

static void Handler(int sig_num) {
  switch (sig_num) {
    case SIGUSR1:
      phase_num = 1;
      break;
    case SIGUSR2:
      phase_num = 2;
      break;
    case SIGTERM:
      exit(0);
      break;
    default:
      signal(sig_num, SIG_DFL);
      break;
  }
}

/**
 * @brief Создает специальную структуру из бинарного файла
 *
 * @param kmeans
 * @param file_name
 * @return int
 */
int CreatPoints(KMeans** kmeans, char const* file_name) {
  if ((kmeans == NULL) || (*kmeans != NULL) || (file_name == NULL)) {
    return FAILURE;
  }

  FILE* fptr = NULL;
  fptr = fopen(file_name, "rb");
  if (fptr == NULL) {
    return FAILURE;
  }
  // HACK: mmap и munmap стоит проверять на ошибку
  KMeans* tmp_kmeans =
      (KMeans*)mmap(NULL, sizeof(KMeans), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANON, -1, 0);
  // В начале файла расположены количество точек и необходимое число кластеров
  fread(&tmp_kmeans->points_cnt, sizeof(size_t), 1, fptr);
  fread(&tmp_kmeans->clusters_cnt, sizeof(size_t), 1, fptr);
  // Кластеров не должно быть больше, чем самих точек
  if (tmp_kmeans->clusters_cnt > tmp_kmeans->points_cnt) {
    munmap(tmp_kmeans, sizeof(KMeans));
    fclose(fptr);
    return FAILURE;
  }
  tmp_kmeans->points = (PointInCluster*)mmap(
      NULL, tmp_kmeans->points_cnt * sizeof(PointInCluster),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  tmp_kmeans->clusters =
      (Point*)mmap(NULL, tmp_kmeans->clusters_cnt * sizeof(Point),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  // Далее расположены сами точки
  for (size_t i = 0; i < tmp_kmeans->points_cnt; ++i) {
    fread(&tmp_kmeans->points[i].point, sizeof(Point), 1, fptr);
  }

  *kmeans = tmp_kmeans;
  fclose(fptr);
  return SUCCESS;
}

/**
 * @brief Отправка сообщения в очередь с ожиданием
 *
 * @param qid
 * @param type
 * @param text
 * @return int
 */
int SendMessage(int qid, long type, char* text) {
  static MsgBuf q_buf;
  q_buf.mtype = type;
  strcpy(q_buf.mtext, text);
  if (msgsnd(qid, &q_buf, strlen(q_buf.mtext) + 1, 0)) {
    return FAILURE;
  }
  return SUCCESS;
}

/**
 * @brief Чтение сообщения из очереди с ожиданием
 *
 * @param qid
 * @param text
 * @param type
 */
void ReadMessage(int qid, char* text, long type) {
  static MsgBuf q_buf;
  q_buf.mtype = type;
  msgrcv(qid, &q_buf, MAX_SEND_SIZE, type, 0);
  strcpy(text, q_buf.mtext);
}

/**
 * @brief Запуск работы дочерних процессов
 * Работает в режиме конечного автомата
 * @param msgid
 * @param kmeans
 */
void StartChildWork(int msgid, KMeans* kmeans) {
  char send_tmp[MAX_SEND_SIZE] = {0};
  char recv_tmp[MAX_SEND_SIZE] = {0};
  // Для синхронизации с родителем
  signal(SIGUSR1, Handler);
  signal(SIGUSR2, Handler);
  signal(SIGTERM, Handler);
  raise(SIGSTOP);
  // Дочерние процессы работают как конечный автомат
  while (1) {
    switch (phase_num) {
      case 1:
        // Фаза 1: сортируем по кластерам
        ReadMessage(msgid, recv_tmp, SORT_MSG);
        size_t start_batch = 0;
        size_t end_batch = 0;
        sscanf(recv_tmp, "%zu %zu", &start_batch, &end_batch);
        printf("Received %zu %zu\n", start_batch, end_batch);
        size_t changed = 0;
        // Тут проверка возврата лишнее, тк всё необходимое же проверено
        ClusterSort(kmeans, start_batch, end_batch, &changed);
        snprintf(send_tmp, MAX_SEND_SIZE, "%zu", changed);
        SendMessage(msgid, TO_PARENT_MSG, send_tmp);
        phase_num = 0;
        break;
      case 2:
        // Фаза 2: Поиск центров кластеров
        ReadMessage(msgid, recv_tmp, CENTER_MSG);
        size_t cnt = 0;
        sscanf(recv_tmp, "%zu", &cnt);
        printf("Received %zu\n", cnt);
        // Тут проверка возврата лишнее, тк всё необходимое же проверено
        FindClusterCenter(kmeans, cnt);
        SendMessage(msgid, TO_PARENT_MSG, "ok");
        phase_num = 0;
        break;
      default:
        break;
    }
  }
  // Сюда 0 вероятность прийти, вырубает процесс родитель
  exit(0);
}

int InitProcesses(KMeans* kmeans,
                  int* msgid,
                  int* pids,
                  size_t const process_cnt) {
  if ((kmeans == NULL) || (pids == NULL) || (msgid == NULL)) {
    return FAILURE;
  }

  // HACK: Стоит проверить успех создания очереди
  *msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0660);
  for (size_t i = 0; i < process_cnt; ++i) {
    pids[i] = fork();
    if (pids[i] == -1) {
      printf("fork failed\n");
      // HACK: Стоит убить уже созданные процессы
      return FAILURE;
    } else if (pids[i] == 0) {
      StartChildWork(*msgid, kmeans);
    } else {
      printf("Created process = %d\n", pids[i]);
    }
  }
  // Ждем, пока каждый процесс закончит свою инициализацию
  for (size_t i = 0; i < process_cnt; ++i) {
    int status = 0;
    waitpid(-1, &status, WUNTRACED);
  }
  // Продолжаем работу всех детей
  for (size_t i = 0; i < process_cnt; ++i) {
    kill(pids[i], SIGCONT);
  }
  return SUCCESS;
}

int PhaseSortClusters(KMeans* kmeans,
                      int const msgid,
                      size_t const process_cnt,
                      int const* pids,
                      size_t* changed) {
  if ((kmeans == NULL) || (pids == NULL) || (changed == NULL)) {
    return FAILURE;
  }

  char send_tmp[MAX_SEND_SIZE] = {0};
  char recv_tmp[MAX_SEND_SIZE] = {0};
  for (size_t i = 0; i < process_cnt; ++i) {
    kill(pids[i], SIGUSR1);
  }
  size_t start_batch = 0;
  size_t end_batch = 0;
  for (size_t i = 0; i < process_cnt; ++i) {
    start_batch = end_batch;
    // Таким подсчетом откусывается всегда "поровну" для всех оставшихся
    // процессов
    end_batch =
        start_batch + (kmeans->points_cnt - start_batch) / (process_cnt - i);
    snprintf(send_tmp, MAX_SEND_SIZE, "%zu %zu", start_batch, end_batch);
    // HACK: Стоит проверить на возврат с ошибкой
    SendMessage(msgid, SORT_MSG, send_tmp);
  }
  size_t changed_tmp = 0;
  *changed = 0;
  for (size_t i = 0; i < process_cnt; ++i) {
    ReadMessage(msgid, recv_tmp, TO_PARENT_MSG);
    sscanf(recv_tmp, "%zu", &changed_tmp);
    *changed += changed_tmp;
  }
  return SUCCESS;
}

int PhaseFindCenter(KMeans* kmeans,
                    int const msgid,
                    size_t const process_cnt,
                    int const* pids) {
  if ((kmeans == NULL) || (pids == NULL)) {
    return FAILURE;
  }

  char send_tmp[MAX_SEND_SIZE] = {0};
  char recv_tmp[MAX_SEND_SIZE] = {0};
  for (size_t i = 0; i < process_cnt; ++i) {
    // HACK: Стоит проверить на возврат с ошибкой
    kill(pids[i], SIGUSR2);
  }
  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    snprintf(send_tmp, MAX_SEND_SIZE, "%zu", i);
    // HACK: Стоит проверить на возврат с ошибкой
    SendMessage(msgid, CENTER_MSG, send_tmp);
  }
  for (size_t i = 0; i < process_cnt; ++i) {
    ReadMessage(msgid, recv_tmp, TO_PARENT_MSG);
  }
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

  size_t process_cnt = kmeans->clusters_cnt;
  int pids[process_cnt];
  int msgid = 0;

  if (InitProcesses(kmeans, &msgid, pids, process_cnt)) {
    return FAILURE;
  }

  size_t changed = 0;
  do {
    // Начинаем фазу 1: сортировки точек по кластерам
    PhaseSortClusters(kmeans, msgid, process_cnt, pids, &changed);
    // Начинаем фазу 2: поиск центра для каждого кластера
    PhaseFindCenter(kmeans, msgid, process_cnt, pids);
  } while (((float)changed / (float)kmeans->points_cnt) > threshold);

  for (size_t i = 0; i < process_cnt; ++i) {
    // HACK: Стоит проверить на возврат с ошибкой
    kill(pids[i], SIGTERM);
    printf("Killed %d\n", pids[i]);
  }
  // HACK: Стоит проверить на возврат с ошибкой
  msgctl(msgid, IPC_RMID, NULL);
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