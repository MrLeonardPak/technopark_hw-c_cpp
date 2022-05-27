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

#include "kmeans_multiprocess.h"

#include <errno.h>

extern int errno;

const float threshold = 0.01;
static int phase_num = 0;

/**
 * @brief Обработчик прерываний
 *
 * @param sig_num
 */
static void Handler(int sig_num) {
  switch (sig_num) {
    case SIGUSR1:
      phase_num = 1;
      break;
    case SIGUSR2:
      phase_num = 2;
      break;
    case SIGTERM:
      phase_num = 3;
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
  KMeans* tmp_kmeans =
      (KMeans*)mmap(NULL, 1 * sizeof(KMeans), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANON, -1, 0);
  if (tmp_kmeans == MAP_FAILED) {
    fclose(fptr);
    return FAILURE;
  }
  // В начале файла расположены количество точек и необходимое число кластеров
  if (fread(&tmp_kmeans->points_cnt, sizeof(size_t), 1, fptr) != 1) {
    fclose(fptr);
    munmap(tmp_kmeans, 1 * sizeof(KMeans));
    return FAILURE;
  };
  if (fread(&tmp_kmeans->clusters_cnt, sizeof(size_t), 1, fptr) != 1) {
    fclose(fptr);
    munmap(tmp_kmeans, 1 * sizeof(KMeans));
    return FAILURE;
  };
  // Кластеров не должно быть больше, чем самих точек
  if (tmp_kmeans->clusters_cnt > tmp_kmeans->points_cnt) {
    fclose(fptr);
    munmap(tmp_kmeans, 1 * sizeof(KMeans));
    return FAILURE;
  }
  tmp_kmeans->points = (PointInCluster*)mmap(
      NULL, tmp_kmeans->points_cnt * sizeof(PointInCluster),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  if (tmp_kmeans->points == MAP_FAILED) {
    fclose(fptr);
    munmap(tmp_kmeans, 1 * sizeof(KMeans));
    return FAILURE;
  }
  tmp_kmeans->clusters =
      (Point*)mmap(NULL, tmp_kmeans->clusters_cnt * sizeof(Point),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  if (tmp_kmeans->clusters == MAP_FAILED) {
    fclose(fptr);
    munmap(tmp_kmeans->points, tmp_kmeans->points_cnt * sizeof(PointInCluster));
    munmap(tmp_kmeans, 1 * sizeof(KMeans));
    return FAILURE;
  }
  // Далее расположены сами точки
  for (size_t i = 0; i < tmp_kmeans->points_cnt; ++i) {
    if (fread(&tmp_kmeans->points[i].point, sizeof(Point), 1, fptr) != 1) {
      fclose(fptr);
      munmap(tmp_kmeans->clusters, tmp_kmeans->clusters_cnt * sizeof(Point));
      munmap(tmp_kmeans->points,
             tmp_kmeans->points_cnt * sizeof(PointInCluster));
      munmap(tmp_kmeans, 1 * sizeof(KMeans));
      return FAILURE;
    };
  }

  *kmeans = tmp_kmeans;
  if (fclose(fptr)) {
    munmap(tmp_kmeans->clusters, tmp_kmeans->clusters_cnt * sizeof(Point));
    munmap(tmp_kmeans->points, tmp_kmeans->points_cnt * sizeof(PointInCluster));
    munmap(tmp_kmeans, 1 * sizeof(KMeans));
    return FAILURE;
  };
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
    if (PhaseSortClusters(kmeans, msgid, process_cnt, pids, &changed)) {
      for (size_t i = 0; i < process_cnt; ++i) {
        kill(pids[i], SIGKILL);
      }
      return FAILURE;
    }

    // Начинаем фазу 2: поиск центра для каждого кластера
    if (PhaseFindCenter(kmeans, msgid, process_cnt, pids)) {
      for (size_t i = 0; i < process_cnt; ++i) {
        kill(pids[i], SIGKILL);
      }
      return FAILURE;
    }
    printf("Changed: %ld\n", changed);
  } while (((float)changed / (float)kmeans->points_cnt) > threshold);
  // Выключаем процессы, удаляем очередь
  for (size_t i = 0; i < process_cnt; ++i) {
    if (kill(pids[i], SIGTERM)) {
      msgctl(msgid, IPC_RMID, NULL);
      return FAILURE;
    }
    printf("Killed %d\n", pids[i]);
  }
  if (msgctl(msgid, IPC_RMID, NULL)) {
    return FAILURE;
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
  int rtn = SUCCESS;
  KMeans* tmp_kmeans = *kmeans;
  if (tmp_kmeans->points != NULL) {
    if (munmap(tmp_kmeans->points,
               tmp_kmeans->points_cnt * sizeof(PointInCluster))) {
      rtn = FAILURE;
    }
    tmp_kmeans->points = NULL;
  }
  if (tmp_kmeans->clusters != NULL) {
    if (munmap(tmp_kmeans->clusters,
               tmp_kmeans->clusters_cnt * sizeof(Point))) {
      rtn = FAILURE;
    }
    tmp_kmeans->clusters = NULL;
  }
  if (munmap(tmp_kmeans, sizeof(KMeans))) {
    rtn = FAILURE;
  }
  tmp_kmeans = NULL;

  *kmeans = tmp_kmeans;
  return rtn;
}

/**
 * @brief Отправка сообщения в очередь с ожиданием
 *
 * @param qid
 * @param type
 * @param text
 * @return int
 */
int SendMessage(int qid, long type, char const* text) {
  if (text == NULL) {
    return FAILURE;
  }
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
 * @return int
 */
int ReadMessage(int qid, char* text, long type) {
  if (text == NULL) {
    return FAILURE;
  }

  static MsgBuf q_buf;
  q_buf.mtype = type;
  // Как вариант с неблокирующим
  // do {
  //   errno = 0;
  //   msgrcv(qid, &q_buf, MAX_SEND_SIZE, type, IPC_NOWAIT);
  // } while (errno == ENOMSG);
  // if (errno > 0) {
  //   return FAILURE;
  // }
  if (msgrcv(qid, &q_buf, MAX_SEND_SIZE, type, 0) == -1) {
    return FAILURE;
  }
  strcpy(text, q_buf.mtext);
  return SUCCESS;
}

/**
 * @brief Запуск работы дочерних процессов
 * Работает в режиме конечного автомата
 * @param msgid
 * @param kmeans
 */
int StartChildWork(int msgid, KMeans* kmeans) {
  if (kmeans == NULL) {
    return FAILURE;
  }

  char send_tmp[MAX_SEND_SIZE] = {0};
  char recv_tmp[MAX_SEND_SIZE] = {0};
  // Для синхронизации с родителем
  signal(SIGUSR1, Handler);
  signal(SIGUSR2, Handler);
  signal(SIGTERM, Handler);
  raise(SIGSTOP);
  // Дочерние процессы работают как конечный автомат
  int work_flag = 1;
  while (work_flag) {
    switch (phase_num) {
      case 1:
        phase_num = 0;
        // Фаза 1: сортируем по кластерам
        ReadMessage(msgid, recv_tmp, SORT_MSG);
        size_t start_batch = 0;
        size_t end_batch = 0;
        sscanf(recv_tmp, "%zu %zu", &start_batch, &end_batch);
        // printf("Received %zu %zu\n", start_batch, end_batch);
        size_t changed = 0;
        // Тут проверка возврата лишнее, тк всё необходимое же проверено
        ClusterSort(kmeans, start_batch, end_batch, &changed);
        snprintf(send_tmp, MAX_SEND_SIZE, "%zu", changed);
        SendMessage(msgid, TO_PARENT_MSG, send_tmp);
        break;
      case 2:
        phase_num = 0;
        // Фаза 2: Поиск центров кластеров
        ReadMessage(msgid, recv_tmp, CENTER_MSG);
        size_t cnt = 0;
        sscanf(recv_tmp, "%zu", &cnt);
        // printf("Received %zu\n", cnt);
        // Тут проверка возврата лишнее, тк всё необходимое же проверено
        FindClusterCenter(kmeans, cnt);
        SendMessage(msgid, TO_PARENT_MSG, "ok");
        break;
      case 3:
        work_flag = 0;
        break;
      default:
        break;
    }
  }
  return SUCCESS;
}

/**
 * @brief Запуск дочерних процессов и синхронизация с ними
 *
 * @param kmeans
 * @param msgid
 * @param pids
 * @param process_cnt
 * @return int
 */
int InitProcesses(KMeans* kmeans,
                  int* msgid,
                  int* pids,
                  size_t const process_cnt) {
  if ((kmeans == NULL) || (pids == NULL) || (msgid == NULL)) {
    return FAILURE;
  }

  *msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0660);
  if (*msgid == -1) {
    return FAILURE;
  }
  for (size_t i = 0; i < process_cnt; ++i) {
    pids[i] = fork();
    if (pids[i] == -1) {
      // Не получается создать нужное количество процессов - завершить все
      for (size_t j = 0; j < i; ++j) {
        kill(pids[j], SIGKILL);
      }
      return FAILURE;
    } else if (pids[i] == 0) {
      exit(StartChildWork(*msgid, kmeans));
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

/**
 * @brief Фаза сортировки точек по кластерам
 *
 * @param kmeans
 * @param msgid
 * @param process_cnt
 * @param pids
 * @param changed
 * @return int
 */
int PhaseSortClusters(KMeans* kmeans,
                      int const msgid,
                      size_t const process_cnt,
                      int const* pids,
                      size_t* changed) {
  if ((kmeans == NULL) || (pids == NULL) || (changed == NULL) ||
      (process_cnt == 0)) {
    return FAILURE;
  }

  char send_tmp[MAX_SEND_SIZE] = {0};
  char recv_tmp[MAX_SEND_SIZE] = {0};
  // Запускаем первую фазу на дочерних процессах
  for (size_t i = 0; i < process_cnt; ++i) {
    kill(pids[i], SIGUSR1);
  }
  size_t start_batch = 0;
  size_t end_batch = 0;
  // Отправляем диапазоны батчей
  for (size_t i = 0; i < process_cnt; ++i) {
    start_batch = end_batch;
    // Таким подсчетом откусывается всегда "поровну" для всех оставшихся
    // процессов
    end_batch =
        start_batch + (kmeans->points_cnt - start_batch) / (process_cnt - i);
    snprintf(send_tmp, MAX_SEND_SIZE, "%zu %zu", start_batch, end_batch);
    if (SendMessage(msgid, SORT_MSG, send_tmp)) {
      return FAILURE;
    }
    // printf("Send %zu %zu\n", start_batch, end_batch);
  }
  size_t changed_tmp = 0;
  *changed = 0;
  // Ожидаем результат от дочерних процессов
  for (size_t i = 0; i < process_cnt; ++i) {
    if (ReadMessage(msgid, recv_tmp, TO_PARENT_MSG)) {
      return FAILURE;
    }
    sscanf(recv_tmp, "%zu", &changed_tmp);
    *changed += changed_tmp;
  }
  return SUCCESS;
}

/**
 * @brief Фаза поиска центра каждого кластера
 *
 * @param kmeans
 * @param msgid
 * @param process_cnt
 * @param pids
 * @return int
 */
int PhaseFindCenter(KMeans* kmeans,
                    int const msgid,
                    size_t const process_cnt,
                    int const* pids) {
  if ((kmeans == NULL) || (pids == NULL) || (process_cnt == 0)) {
    return FAILURE;
  }

  char send_tmp[MAX_SEND_SIZE] = {0};
  char recv_tmp[MAX_SEND_SIZE] = {0};
  // Запускаем вторую фазу на дочерних процессах
  for (size_t i = 0; i < process_cnt; ++i) {
    if (kill(pids[i], SIGUSR2)) {
      return FAILURE;
    }
  }
  // Передаем номер кластера
  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    snprintf(send_tmp, MAX_SEND_SIZE, "%zu", i);
    if (SendMessage(msgid, CENTER_MSG, send_tmp)) {
      return FAILURE;
    }
    // printf("Send %zu\n", i);
  }
  // Ожидаем ответ от дочерних процессов
  for (size_t i = 0; i < process_cnt; ++i) {
    if (ReadMessage(msgid, recv_tmp, TO_PARENT_MSG)) {
      return FAILURE;
    }
  }
  return SUCCESS;
}