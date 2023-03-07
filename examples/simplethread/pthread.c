#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <sys/utsname.h>

// The assignment requires this set to 1
#define NUM_THREADS 12
#define PREFIX "[COURSE:1][ASSIGNMENT:1]"

typedef struct {
    int threadIdx;
} threadParams_t;

// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

void logUname() {
    struct utsname unameData;
    uname(&unameData);
    printf("%s %s %s %s %s\n", unameData.sysname, unameData.nodename, unameData.release, unameData.version,
        unameData.machine);
    syslog(LOG_CRIT, "%s %s %s %s %s %s\n", PREFIX, unameData.sysname, unameData.nodename, unameData.release,
        unameData.version, unameData.machine);
}

void *counterThread(void *threadp) {
    int sum = 0;
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for (i = 1; i < (threadParams->threadIdx) + 1; ++i) {
        sum = sum + i;
    }

    syslog(LOG_CRIT, "%s %s\n", PREFIX, "Hello World from Thread!");
    printf("Thread idx=%d, sum[0...%d]=%d\n", threadParams->threadIdx, threadParams->threadIdx, sum);
}

int main(int argc, char *argv[]) {
    int rc;
    int i;

    logUname();

    for (i = 0; i < NUM_THREADS; ++i) {
        threadParams[i].threadIdx = i;

        pthread_create(&threads[i],       // pointer to thread descriptor
            (void *)0,                    // use default attributes
            counterThread,                // thread function entry point
            (void *)&(threadParams[i]));  // parameters to pass in
    }

    syslog(LOG_CRIT, "%s %s\n", PREFIX, "Hello World from Main!");

    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("TEST COMPLETE\n");
}
