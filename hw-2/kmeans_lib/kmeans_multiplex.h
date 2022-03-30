#ifndef HW2_KMEANS_MULTIPLEX
#define HW2_KMEANS_MULTIPLEX

#define MAX_SEND_SIZE 80
#define SORT_MSG 100
#define CENTER_MSG 200

typedef struct MsgBuf {
  long mtype;
  char mtext[MAX_SEND_SIZE];
} MsgBuf;

#endif  // HW2_KMEANS_MULTIPLEX