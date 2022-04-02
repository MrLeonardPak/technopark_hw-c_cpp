#ifndef HW2_KMEANS_MULTIPROCESS
#define HW2_KMEANS_MULTIPROCESS

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

int SendMessage(int qid, long type, char const* text);
int ReadMessage(int qid, char* text, long type);
int StartChildWork(int msgid, KMeans* kmeans);
int InitProcesses(KMeans* kmeans,
                  int* msgid,
                  int* pids,
                  size_t const process_cnt);
int PhaseSortClusters(KMeans* kmeans,
                      int const msgid,
                      size_t const process_cnt,
                      int const* pids,
                      size_t* changed);
int PhaseFindCenter(KMeans* kmeans,
                    int const msgid,
                    size_t const process_cnt,
                    int const* pids);
#endif  // HW2_KMEANS_MULTIPROCESS
