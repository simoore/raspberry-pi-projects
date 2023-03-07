#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#define UNAME_STR "Linux raspberrypi 5.15.84-v7l+ #1613 SMP Thu Jan 5 12:01:26 GMT 2023 armv7l GNU/Linux"
#define PREFIX "[COURSE:1][ASSIGNMENT:3]"
#define NTHREADS 128
#define SCHED_POLICY SCHED_FIFO

typedef struct {
    int threadIdx;
} threadParams_t;

static pthread_attr_t sFifoSchedAttr;
static pthread_t sThreads[NTHREADS];
static threadParams_t sThreadParams[NTHREADS];
static struct sched_param sFifoParam;

void printScheduler(void) {
    // The system call `sched_getscheduler` returns the index of the scheduler being used for this process.
    int schedType = sched_getscheduler(getpid());

    switch (schedType) {
    case SCHED_FIFO:
        printf("Pthread policy is SCHED_FIFO\n");
        break;
    case SCHED_OTHER:
        printf("Pthread policy is SCHED_OTHER\n");
        break;
    case SCHED_RR:
        printf("Pthread policy is SCHED_RR\n");
        break;
    default:
        printf("Pthread policy is UNKNOWN\n");
    }
}

void setScheduler(void) {
    printf("Current Scheduler\n");
    printScheduler();

    // We first initialize the thread attribute object. This sets the default values of the attributes.
    // The inherit-scheduler attribute indicates whether a new thread should take its attributes from the creating
    // thread or the scheduler. In our case we want them to come from the scheduler.
    // We are setting the sched policy to SCHED_FIFO which is a preempting scheduler that uses a FIFO policy is a
    // thread has the same priority.
    pthread_attr_init(&sFifoSchedAttr);
    pthread_attr_setinheritsched(&sFifoSchedAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&sFifoSchedAttr, SCHED_POLICY);

    // This is used to setup the affinity of the threads created by the scheduler. We are only allowing threads in
    // this process to run on core 3.
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    pthread_attr_setaffinity_np(&sFifoSchedAttr, sizeof(cpu_set_t), &cpuset);

    sFifoParam.sched_priority = sched_get_priority_max(SCHED_POLICY);

    // This sets the scheduler for this process.
    int rc = sched_setscheduler(getpid(), SCHED_POLICY, &sFifoParam);
    if (rc < 0) {
        perror("sched_setscheduler");
    }

    pthread_attr_setschedparam(&sFifoSchedAttr, &sFifoParam);

    printf("New Scheduler\n");
    printScheduler();
}

void *sumThread(void *threadp) {
    int sum = 0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for (int i = 1; i <= threadParams->threadIdx; ++i) {
        sum = sum + i;
    }
    syslog(LOG_CRIT, "%s Thread idx=%d, sum[1..%d]=%d Running on core: %d\n", PREFIX, threadParams->threadIdx,
        threadParams->threadIdx, sum, sched_getcpu());
    return NULL;
}

int main(int argc, char *argv[]) {
    setScheduler();
    syslog(LOG_CRIT, "%s %s\n", PREFIX, UNAME_STR);

    // Print some information about the scheduler.
    cpu_set_t cpuset;
    pthread_t mainthread = pthread_self();
    int rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        perror("pthread_getaffinity_np");
    } else {
        printf("main thread running on CPU=%d, CPUs=", sched_getcpu());
        for (int i = 0; i < CPU_SETSIZE; ++i) {
            if (CPU_ISSET(i, &cpuset)) {
                printf(" %d", i);
            }
        }
        printf("\n");
    }

    for (int i = 0; i < NTHREADS; ++i) {
        sThreadParams[i].threadIdx = i;
        pthread_create(&sThreads[i], &sFifoSchedAttr, sumThread, (void *)&(sThreadParams[i]));
    }

    for (int i = 0; i < NTHREADS; ++i) {
        pthread_join(sThreads[i], NULL);
    }

    printf("TEST COMPLETE\n");
}
