#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>

typedef struct queueNode{
    char* data;
    int empty;
} Node;

typedef struct queueStruct{
    Node* array;
    int first;
    int last;
    int size;
} Queue;

int queueCreate(Queue *q);

int queueEmpty(Queue *q);

int queueFull(Queue *q);

int queuePush(Queue *q, char* data);

char* queuePop(Queue *q);

void queueClean(Queue *q);

#endif

