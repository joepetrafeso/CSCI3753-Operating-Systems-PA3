#include<pthread.h>

typedef struct threadStruct {
    Queue *q;
    FILE **infiles;
    int *inputSize;
    int *nextInput;
    int *inputCount;
    FILE *outfile;
    pthread_mutex_t *buffMutex;
    pthread_mutex_t *outMutex;
    pthread_cond_t *cond[2];
    int *exit;
}thread;

void *requester(void *info);

void *resolver(void *info);
