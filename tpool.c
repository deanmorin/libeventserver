#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>


int tPoolInit(tPool** tpoolp, int numWorkerThreads, int maxQueueSize,
        int blockWhenQueueFull)
{
    int i = 0;
    int rtn = 0;
    tPool* tpool = NULL;

    /* allocate the pool data struct */
    if ((tpool = (tPool*) malloc(sizeof(tPool))) == NULL)
    {
        return TPOOL_ERR_MALLOC;
    }

    /* initialize the fields */
    tpool->numThreads = numWorkerThreads;
    tpool->maxQueueSize = maxQueueSize;
    tpool->blockWhenQueueFull = blockWhenQueueFull;
    if ((tpool->threads = (pthread_t*) malloc(
            sizeof(pthread_t) * numWorkerThreads)) == NULL)
    {
        return TPOOL_ERR_MALLOC;
    }
    tpool->queueSize = 0;
    tpool->queueHead = NULL;
    tpool->queueTail = NULL;
    tpool->queueClosed = 0;
    tpool->shutdown = 0;


    if ((rtn = pthread_mutex_init(&(tpool->queueLock), NULL)) != 0)
    {
        return TPOOL_ERR_MUTEX_INIT;
    }

    if ((rtn = pthread_cond_init(&(tpool->queueNotEmpty), NULL)) != 0)
    {
        return TPOOL_ERR_COND_INIT;
    }

    if ((rtn = pthread_cond_init(&(tpool->queueNotFull), NULL)) != 0)
    {
        return TPOOL_ERR_COND_INIT;
    }

    if ((rtn = pthread_cond_init(&(tpool->queueEmpty), NULL)) != 0)
    {
        return TPOOL_ERR_COND_INIT;
    }

    /* create the thread pool, one thread at a time */
    for (i = 0; i < numWorkerThreads; i++)
    {
        if ((rtn = pthread_create(&(tpool->threads[i]), NULL, tPoolThreadDoJobs, 
                           (void*) tpool)) != 0)
        {
            return TPOOL_ERR_CREATE_THREAD;
        }
    }

    *tpoolp = tpool;   
    return 0;
}

int tPoolAddJob(tPool* tpool, void (*routine)(void*), void* arg)
{
    tPoolJob* newJob = NULL;
    pthread_mutex_lock(&tpool->queueLock);

    if ((tpool->queueSize == tpool->maxQueueSize)
        && !tpool->blockWhenQueueFull)
    {
        pthread_mutex_unlock(&tpool->queueLock);
        return TPOOL_QUEUE_FULL;
    }

    while ((tpool->queueSize == tpool->maxQueueSize) 
           && (!(tpool->shutdown || tpool->queueClosed)))
    {
        pthread_cond_wait(&tpool->queueNotFull, &tpool->queueLock);
    }

    if (tpool->shutdown || tpool->queueClosed)
    {
        pthread_mutex_unlock(&tpool->queueLock);
        return TPOOL_SHUTDOWN;
    }

    /* allocate the data structure for work */
    newJob = (tPoolJob*) malloc(sizeof(tPoolJob));
    newJob->routine = routine;
    newJob->arg = arg;
    newJob->next = NULL;

    if (tpool->queueSize == 0)
    {
        tpool->queueTail = tpool->queueHead = newJob;
    }
    else
    {
        (tpool->queueTail)->next = newJob;
        tpool->queueTail = newJob;
    }

    tpool->queueSize++;
    pthread_cond_signal(&tpool->queueNotEmpty);
    pthread_mutex_unlock(&tpool->queueLock);
    return 0;
}

int tPoolDestroy(tPool* tpool, int finishQueue)
{
    int i = 0;
    tPoolJob* cur_nodep = NULL;

    if (pthread_mutex_lock(&(tpool->queueLock)) != 0)
    {
        return TPOOL_ERR_MUTEX;
    }

    /* Here we check if the shutdown process is already in progress */
    if (tpool->queueClosed || tpool->shutdown)
    {
        pthread_mutex_lock(&(tpool->queueLock));

        return 0;
    }

    tpool->queueClosed = 1;

    /* if the finish flag has need already set, we need to wait for workers to 
     * drain the queue */
    if (finishQueue)
    {
        while(tpool->queueSize != 0)
        {
            if (pthread_cond_wait(&(tpool->queueEmpty),
                                  &(tpool->queueLock)) != 0)
            {
                pthread_mutex_unlock(&tpool->queueLock);
                return TPOOL_ERR_COND_WAIT;
            }
        }
    }

    tpool->shutdown = 1;

    /*unlock the queue mutex */
    if (pthread_mutex_unlock(&(tpool->queueLock)) != 0)
    {
        return TPOOL_ERR_MUTEX;
    }

    /* now we need to wake up all the worker threads so they check the shutdown
     * flag and shut themselves down */
    if (pthread_cond_broadcast(&(tpool->queueNotEmpty)) != 0)
    {
        return TPOOL_ERR_COND_BROAD;
    }

    if (pthread_cond_broadcast(&(tpool->queueNotFull)) != 0)
    {
        return TPOOL_ERR_COND_BROAD;
    }

    /* wait for all worker threads to exit, then free them */
    for (i = 0; i < tpool->numThreads; i++)
    {
        if (pthread_join(tpool->threads[i],NULL) != 0)
        {
            return TPOOL_ERR_THREAD_JOIN;
        }
    }

    /* now we need to cleanup and free all the thread pool structs here */

    free(tpool->threads);

    while (tpool->queueHead != NULL)
    {
        cur_nodep = tpool->queueHead->next;
        tpool->queueHead = tpool->queueHead->next;
        free(cur_nodep);
    }

    /* release the mutex and condition variables */
    pthread_mutex_destroy(&(tpool->queueLock));
    pthread_cond_destroy(&(tpool->queueNotEmpty));
    pthread_cond_destroy(&(tpool->queueNotFull));
    pthread_cond_destroy(&(tpool->queueEmpty));

    free(tpool);

    return 0;
}

void* tPoolThreadDoJobs(void* tpool)
{
    tPoolJob* job = NULL;
    tPool* tpoolp = (tPool*) tpool;

    while (1)
    {
        pthread_mutex_lock(&(tpoolp->queueLock));
        while (tpoolp->queueSize == 0 && !tpoolp->shutdown)
        {
            /*fprintf(stderr, "%lu - waiting for a job\n", (unsigned long) pthread_self());*/
            pthread_cond_wait(&(tpoolp->queueNotEmpty),&(tpoolp->queueLock));
        }

        if (tpoolp->shutdown)
        {
            /*fprintf(stderr, "%lu - shutting down\n", (unsigned long) pthread_self());*/
            pthread_mutex_unlock(&(tpoolp->queueLock));
            pthread_exit(NULL);    
        }

        job = tpoolp->queueHead;
        tpoolp->queueSize--;
        
        if (tpoolp->queueSize == 0) 
        {
            /*fprintf(stderr, "%lu - queue is empty\n", (unsigned long) pthread_self());*/
            tpoolp->queueHead = tpoolp->queueTail = NULL;
            pthread_cond_signal(&(tpoolp->queueEmpty));
        }
        else
        {
            /*fprintf(stderr, "%lu - next job\n", (unsigned long) pthread_self());*/
            tpoolp->queueHead = job->next;
        }

        if (tpoolp->blockWhenQueueFull
            && tpoolp->queueSize == tpoolp->maxQueueSize - 1)
        {
            /*fprintf(stderr, "%lu - queue no longer full\n", (unsigned long) pthread_self());*/
            pthread_cond_signal(&(tpoolp->queueNotFull));
        }

        pthread_mutex_unlock(&(tpoolp->queueLock));
        (*(job->routine))(job->arg);
        /*fprintf(stderr, "\tqueue size: %d\n", tpoolp->queueSize);*/
        free(job);
    }
}

