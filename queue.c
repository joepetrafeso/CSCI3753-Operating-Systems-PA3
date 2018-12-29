#include <stdlib.h>
#include <string.h>
#include "queue.h"

int queueCreate(Queue *q)
{
    q->size=10;
    char *temp;
    int i;
    q->array=malloc(sizeof(Node)*q->size);

    for(i=0;i<q->size;i++)
    {
        temp=malloc(1025);
        q->array[i].data=temp;
        q->array[i].empty=1;
    }

    q->first=0;
    q->last=0;
    return q->size;
}

int queueEmpty(Queue *q)
{
    if(q->first==q->last && q->array[q->first].empty==1)
        return 1;
    else
        return 0;
}

int queueFull(Queue *q)
{
    if(q->first==q->last && q->array[q->first].empty==0)
        return 1;
    else
        return 0;
}

int queuePush(Queue *q, char* data)
{
    if(queueFull(q)==1)
        return -1;
    //q->array[q->last].data=NULL;  possible bug
    strncpy(q->array[q->last].data,data,1025);
    q->array[q->last].empty=0;
    q->last=(q->last+1) % q->size;

    return 0;
}

char* queuePop(Queue *q)
{
    if(queueEmpty(q)==1)
        return NULL;

    char* temp=q->array[q->first].data;
    q->array[q->first].empty=1;
    q->first=(q->first+1) %q->size;
    return temp;
}

void queueClean(Queue *q)
{
    int i;
    while(!queueEmpty(q)){
        queuePop(q);
    }
    for(i=0;i<q->size;i++)
        free(q->array[i].data);

    free(q->array);
}
