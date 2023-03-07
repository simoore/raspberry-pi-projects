#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#define UNAME_STR "Linux raspberrypi 5.15.84-v7l+ #1613 SMP Thu Jan 5 12:01:26 GMT 2023 armv7l GNU/Linux"
#define PREFIX "[COURSE:1][ASSIGNMENT:2]"
#define NTHREADS 128
#define COUNT 10

typedef struct {
    int threadIdx;
} threadParams_t;

// POSIX thread declarations and scheduling attributes
pthread_t threads[NTHREADS];
threadParams_t threadParams[NTHREADS];

void *sumThread(void *threadp) {
    int sum = 0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for (int i = 1; i <= COUNT; ++i) {
        sum = sum + i;   
    }
    syslog(LOG_CRIT, "%s Thread idx=%d, sum[1..10]=%d\n", PREFIX, threadParams->threadIdx, sum);
}

int main(int argc, char *argv[]) {
    int i;

    syslog(LOG_CRIT, "%s %s\n", PREFIX, UNAME_STR);

    for (i = 0; i < NTHREADS; ++i) {
        threadParams[i].threadIdx = i;
        pthread_create(&threads[i], (void *)0, sumThread, (void *)&(threadParams[i]));
    }
    
    for (i = 0; i < NTHREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("TEST COMPLETE\n");
}
