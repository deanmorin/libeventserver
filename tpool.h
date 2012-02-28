#ifndef TANTALUS_TPOOL_H
#define TANTALUS_TPOOL_H

#include <pthread.h>
#include <sys/types.h>

/* tPoolInit return codes */
#define TPOOL_ERR_MALLOC            101
#define TPOOL_ERR_MUTEX_INIT        102
#define TPOOL_ERR_COND_INIT         103
#define TPOOL_ERR_CREATE_THREAD     104
/* tPoolAddJob return codes */
#define TPOOL_QUEUE_FULL            201
#define TPOOL_SHUTDOWN              202
/* tPoolDestroy return codes */
#define TPOOL_ERR_MUTEX             301
#define TPOOL_ERR_COND_WAIT         302
#define TPOOL_ERR_COND_BROAD        303
#define TPOOL_ERR_THREAD_JOIN       304


/**
 * A job that can be added to a job queue in order to be processed by a worker
 * thread.
 */
typedef struct tPoolJob_struct
{
    /** Function that the worker thread will run. */ 
    void(*routine)(void*);
    /** Argument that will be passed to the worker thread. */
    void* arg;
    /** The next job in the job queue. */
    struct tPoolJob_struct* next;

} tPoolJob;

/**
 * The tPool structure represents a thread pool. A tPool should be initialized
 * by calling tpoolInit(). Jobs can then be added with tpoolAddJob().
 */
typedef struct
{
    /* thread pool characteristics */

    /** Number of worker threads in the pool. */
    int32_t numThreads;
    /** The maximum number of pending jobs in the job queue. */
    int32_t maxQueueSize;
    /** True if tpool_add_work() should block if the queue is full. */
    int32_t blockWhenQueueFull;      

    /* thread pool state */
    
    /** Number of jobs in the queue. */
    int32_t queueSize;
    /** A non-zero value means the job queue will not accept new jobs. */
    int32_t queueClosed;
    /** A non-zero value means the queue is shutting down. */
    int32_t shutdown;
    /** First job in the job queue. */
    tPoolJob* queueHead;
    /** Last job in the job queue. */
    tPoolJob* queueTail;
    /** The mutex used by this pool. */
    pthread_mutex_t queueLock;
    /** Indicates when the queue is empty. */
    pthread_cond_t queueNotEmpty;
    /** Indicates when the queue is not empty. */
    pthread_cond_t queueNotFull;
    /** Indicates when the queue is empty. */
    pthread_cond_t queueEmpty;
    /** The worker threads. */
    pthread_t* threads;

} tPool;


/**
 * Creates and initializes a thread pool.
 *
 * @author Dean Morin (based on work by Igor Cheifot)
 * @param tpoolp A double pointer to a tpool struct that will be initialized.
 *      It should be a null pointer; memory for the struct will be allocated
 *      within the function.
 * @param num_worker_threads The number of worker threads that the pool should
 *      have.
 * @param maxQueueSize The maximum number of jobs allowed in the queue.
 * @param blockWhenQueueFull 0 if a call to tpool_add_work() should not block
 *      when the job queue is full, any other value for blocking mode.
 * @return 0 on success.
 */
int tPoolInit(tPool** tpoolp, int numWorkerThreads, int maxQueueSize,
        int blockWhenQueueFull);

/**
 * Adds a job to the job queue to eventually be completed by a worker thread.
 *
 * @author Dean Morin (based on work by Igor Cheifot)
 * @param tpool The thread pool that the job should be added to.
 * @param routine The function that the worker thread will run.
 * @param arg The argument that will be passed to the worker thread.
 * @return 1 on a job being successfully added to the queue
 */
int tPoolAddJob(tPool* tpool, void (*routine)(void*), void* arg);

/**
 * Destroys a thread pool.
 *
 * @author Dean Morin (based on work by Igor Cheifot)
 * @param tpool The thread pool that needs to be shut down.
 * @param finishQueue Set to non-zero if the jobs currently in the queue should
 *      be finished before shutting down
 * @return
 */
int tPoolDestroy(tPool* tpool, int finishQueue);

/**
 * The threads created in tPoolInit() run this function. This function runs in a 
 * forever loop until it shuts down. It will block when the job queue is empty. 
 * When a job is put in the queue, one of the blocked threads will resume and 
 * process the job. This does not ever need to be called from outside of
 * tPoolInit()
 *
 * @author Dean Morin (based on work by Igor Cheifot)
 * @param tpool The thread pool that the thread belongs to.
 */
void* tPoolThreadDoJobs(void* tpool);

#endif

