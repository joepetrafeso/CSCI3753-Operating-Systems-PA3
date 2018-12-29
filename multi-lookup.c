#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "util.h"
#include <string.h>
#include "queue.h"
#include "multi-lookup.h"
#include<sys/syscall.h>
#include<unistd.h>
#include <sys/time.h>



void *requester(void *info)
{
    thread *thread=info;
    Queue *q=thread->q;
    pthread_mutex_t *bufferMutex=thread->buffMutex;
    pthread_mutex_t *inWriteMutex=thread->outMutex;
    pthread_cond_t *condreq=thread->cond[0];
    pthread_cond_t *condres=thread->cond[1];
    int *nextIn=thread->nextInput;
    int *countIn=thread->inputCount;
    char name[1025];
    pid_t tid;
    tid = syscall(SYS_gettid);
    int count=0;
    int i=0;

    while(*countIn<*thread->inputSize)
    {

        FILE *current=thread->infiles[*nextIn];
        if(current==NULL)
        {
            *nextIn=(*nextIn+1) % *thread->inputSize;
            current=thread->infiles[*nextIn];
        }
        *nextIn=(*nextIn+1) % *thread->inputSize;
        while(fscanf(current,"%1024s", name)>0)
        {
            if(i==0)
                count++;
            i=1;
            pthread_mutex_lock(bufferMutex);
            while(queueFull(q))
                pthread_cond_wait(condreq,bufferMutex);

            if(!queueFull(q))
            {
                queuePush(q,name);
                //printf("%d Pushing: %s\n",tid,name);

                pthread_cond_signal(condres);
                pthread_mutex_unlock(bufferMutex);
            }
        }
        if(i==0)
        {
            *countIn=*countIn+1;
        }
        i=0;
    }
    pthread_mutex_lock(inWriteMutex);
    fprintf(thread->outfile, "Thread %d serviced %d files.\n", tid, count);
    pthread_mutex_unlock(inWriteMutex);

}

void *resolver(void *info)
{
    int dns;
    char *name;
    char* site;
    char ipname[INET6_ADDRSTRLEN];
    thread *thread=info;
    int *noReq=thread->exit;
    Queue *q=thread->q;
    pthread_mutex_t *bufferMutex=thread->buffMutex;
    pthread_mutex_t *writeMutex=thread->outMutex;
    pthread_cond_t *condreq=thread->cond[0];
    pthread_cond_t *condres=thread->cond[1];
    pid_t tid;
    tid = syscall(SYS_gettid);

    while(!queueEmpty(q) || !*noReq)
    {
        pthread_mutex_lock(bufferMutex);
        while(queueEmpty(q))
            pthread_cond_wait(condres,bufferMutex);

        if(!queueEmpty(q))
        {
            name=queuePop(q);
            site=malloc(1025);
            strncpy(site,name,1025);
            //printf("%d popping %s\n",tid,site);
            pthread_cond_signal(condreq);
            pthread_mutex_unlock(bufferMutex);
            dns=dnslookup(site,ipname,sizeof(ipname));
            if(dns==-1)
                strncpy(ipname,"",sizeof(ipname));

            pthread_mutex_lock(writeMutex);
            fprintf(thread->outfile, "%s,%s\n", site, ipname);
            pthread_mutex_unlock(writeMutex);
            free(site);
        }
    }


}

int main(int argc, char* argv[])
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    long long millisecondStart = start.tv_sec*1000LL + start.tv_usec/1000;
    pthread_mutex_t bufferMutex;
    pthread_mutex_t writeMutex;
    pthread_mutex_t inWriteMutex;
    pthread_cond_t condres, condreq;
    int numReq=atoi(argv[1]);
    int numRes=atoi(argv[2]);
    if(numReq>10)
    {
        printf("Error: Program can run with at most 10 requester threads\n");
        exit(1);
    }
    if(numRes>5)
    {
        printf("Error: Program can run with at most 5 resolver threads\n");
        exit(1);
    }
    pthread_t th1[numReq], th2[numRes];
    Queue q;
    int resExit=0;
    thread reqInfo;
    thread resInfo;
    int queueSize;
    int inputNum=argc-5;
    if(inputNum>10)
    {
        printf("Error: Program can run with at most 10 input files\n");
        exit(1);
    }
    int nextIn=0;
    int countIn=0;
    int i;

    pthread_mutex_init(&bufferMutex, NULL);
    pthread_mutex_init(&writeMutex,NULL);
    pthread_mutex_init(&inWriteMutex,NULL);
    pthread_cond_init(&condreq, NULL);		/* Initialize consumer condition variable */
    pthread_cond_init(&condres, NULL);		/* Initialize producer condition variable */
    queueCreate(&q);

    FILE *inFile[inputNum];
    FILE *outFile;
    FILE *reqOutFile;
    for(i=0;i<inputNum;i++){
        if ((inFile[i] = fopen(argv[i+5], "r")) == NULL)
        {
            printf("Error: incorrect input file path\n");

        }
    }
    if ((outFile = fopen(argv[4], "w")) == NULL)
    {
        printf("Error: incorrect output file path\n");
        exit(1);
    }
    if ((reqOutFile = fopen(argv[3], "w")) == NULL)
    {
        printf("Error: incorrect output file path\n");
        exit(1);
    }

    reqInfo.q=&q;
    reqInfo.infiles=inFile;
    reqInfo.outfile=reqOutFile;
    reqInfo.buffMutex=&bufferMutex;
    reqInfo.outMutex=&inWriteMutex;
    reqInfo.cond[0]=&condreq;
    reqInfo.cond[1]=&condres;
    reqInfo.exit=&resExit;
    reqInfo.inputSize=&inputNum;
    reqInfo.nextInput=&nextIn;
    reqInfo.inputCount=&countIn;

    resInfo.q=&q;
    resInfo.outfile=outFile;
    resInfo.buffMutex=&bufferMutex;
    resInfo.outMutex=&writeMutex;
    resInfo.cond[0]=&condreq;
    resInfo.cond[1]=&condres;
    resInfo.exit=&resExit;

    for(i=0;i<numReq;i++)
    {
        if (pthread_create(&th1[i], NULL, requester, (void *)&reqInfo)) {
            perror("error creating the first thread");
            exit(1);
        }
    }

    for(i=0;i<numRes;i++)
    {
        if (pthread_create(&th2[i], NULL, resolver, (void *)&resInfo)) {
            perror("error creating the second thread");
            exit(1);
        }
    }

    for(i=0;i<numReq;i++)
        pthread_join(th1[i], 0);

    resExit=1;

    for(i=0;i<numRes;i++)
        pthread_join(th2[i],0);
    queueClean(&q);
    for(i=0;i<inputNum;i++)
    {
        if(inFile[i]!=NULL)
            fclose(inFile[i]);
    }
    fclose(outFile);
    fclose(reqOutFile);
    gettimeofday(&end, NULL);
    long long millisecondEnd = end.tv_sec*1000LL + end.tv_usec/1000;
    printf("Total runtime: %lld milliseconds\n",millisecondEnd-millisecondStart);
    return 0;
}
