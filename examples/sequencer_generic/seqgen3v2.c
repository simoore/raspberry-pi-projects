// Sequencer Generic Demonstration
//
// The purpose of this code is to provide an example for how to best sequence a set of periodic services in Linux user
// space without specialized hardware like an auxiliary programmable interval timere and/or real-time clock. For
// problems similar to and including the final project in real-time systems.
//
// AMP Configuration (check core status with "lscpu"):
//
// 1) Uses SCEHD_FIFO - https://man7.org/linux/man-pages//man7/sched.7.html
// 2) Sequencer runs on core 1
// 3) Three services run on core 2
// 5) Linux kernel mostly runs on core 0, but does load balance non-RT workload over all cores
// 6) check for irqbalance [https://linux.die.net/man/1/irqbalance] which also distribute IRQ handlers
//
// What we really want in addition to SCHED_FIFO with CPU core affinity is:
//
// 1) A reliable periodic source of interrupts (emulated by delay in a loop here)
// 2) An accurate (minimal drift) and precise timestamp
//    * e.g. accurate to 1 millisecond or less, ideally 1 microsecond, but not realistic on an RTOS even
//    * overall, what we want is predictable response with some accuracy (minimal drift) and precision
//
// Linux user space presents a challenge because:
//
// 1) Accurate timestamps are either not available or the ASM instructions to read system clocks can't
//    be issued in user space for security reasons (x86 and x64 TSC, ARM STC).
// 2) User space time with clock_gettime is recommended, but still requires the overhead of a system call
// 3) Linux user space is inherently driven by the jiffy and tick as shown by:
//    * "getconf CLK_TCK" - normall 10 msec tick at 100 Hz
//    * cat /proc/timer_list
// 4) Linux kernel space certainly has more accurate timers that are high resolution, but we would have to
//    write our entire solution as a kernel module and/or use custom kernel modules for timekeeping and
//    selected services.
// 5) Linux kernel patches for best real-time performance include RT PREEMPT (http://www.frank-durr.de/?p=203)
// 6) MUTEX semaphores can cause unbounded priority inversion with SCHED_FIFO, so they should be avoided or
//    * use kernel patches for RT semaphore support
//      [https://opensourceforu.com/2019/04/how-to-avoid-priority-inversion-and-enable-priority-inheritance-in-linux-kernel-programming/]
//    * use the FUTEX instead of standard POSIX semaphores
//      [https://eli.thegreenplace.net/2018/basics-of-futexes/]
//    * POSIX sempaphores do have inversion safe features, but they do not work on un-patched Linux distros
//
// However, for our class goals for soft real-time synchronization with a 1 Hz and a 10 Hz external
// clock (and physical process), the user space approach should provide sufficient accuracy required and
// precision which is limited by our camera frame rate to 30 Hz anyway (33.33 msec).
//
// Sequencer - 100 Hz, gives semaphores to all other services
// Service_1 - 50 Hz, every other Sequencer loop
// Service_2 - 10 Hz, every 10th Sequencer loop
// Service_3 - 6.66 Hz, every 15th Sequencer loop
//
// With the above, priorities by RM policy would be:
//
// Sequencer = RT_MAX	@ 100 Hz
// Servcie_1 = RT_MAX-1	@ 50  Hz
// Service_2 = RT_MAX-2	@ 10  Hz
// Service_3 = RT_MAX-3	@ 6.66 Hz

// This is necessary for CPU affinity macros in Linux
#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <sys/sysinfo.h>
#include <sys/time.h>

#define USEC_PER_MSEC (1000)
#define NANOSEC_PER_MSEC (1000000)
#define NANOSEC_PER_SEC (1000000000)
#define SEQUENCER_CORE (1)
#define SERVICE_CORE (2)
#define TRUE (1)
#define FALSE (0)
#define NUM_THREADS (3)

// Of the available user space clocks, CLOCK_MONONTONIC_RAW is typically most precise and not subject to
// updates from external timer adjustments
//
// However, some POSIX functions like clock_nanosleep can only use adjusted CLOCK_MONOTONIC or CLOCK_REALTIME
//
// #define MY_CLOCK_TYPE CLOCK_REALTIME
// #define MY_CLOCK_TYPE CLOCK_MONOTONIC
#define MY_CLOCK_TYPE CLOCK_MONOTONIC_RAW
// #define MY_CLOCK_TYPE CLOCK_REALTIME_COARSE
// #define MY_CLOCK_TYPE CLOCK_MONTONIC_COARSE

static int sAbortTest = FALSE;
static struct timespec sStartTimeVal;
static double sStartRealtime;
static uint64_t sSequencePeriods;
static uint64_t sSeqCnt = 0;

static timer_t sTimer1;
static struct itimerspec sItime = {{1, 0}, {1, 0}};
static struct itimerspec sLastItime;
static int sPeriod[] = {2, 10, 15};

static sem_t sSem[NUM_THREADS];
static int sAbort[NUM_THREADS] = {FALSE};

typedef struct {
    int threadIdx;
} threadParams_t;

void *service(void *threadp);
void sequencer(int id);
double getTimeMsec(void);
double realtime(struct timespec *tsptr);
void printScheduler(void);

// For background on high resolution time-stamps and clocks:
//
// 1) https://www.kernel.org/doc/html/latest/core-api/timekeeping.html
// 2) https://blog.regehr.org/archives/794 - Raspberry Pi
// 3) https://blog.trailofbits.com/2019/10/03/tsc-frequency-for-all-better-profiling-and-benchmarking/
// 4) http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/example-1/perfmon.c
// 5) https://blog.remibergsma.com/2013/05/12/how-accurately-can-the-raspberry-pi-keep-time/
//
// The Raspberry Pi does not ship with a TSC nor HPET counter to use as clocksource. Instead it relies on
// the STC that Raspbian presents as a clocksource. Based on the source code, “STC: a free running counter
// that increments at the rate of 1MHz”. This means it increments every microsecond.
//
// "sudo apt-get install adjtimex" for an interesting utility to adjust your system clock

void *service(void *threadp) {
    struct timespec currentTimeVal;
    double currentRealtime;
    uint64_t cnt = 0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    // Start up processing and resource initialization.
    clock_gettime(MY_CLOCK_TYPE, &currentTimeVal);
    currentRealtime = realtime(&currentTimeVal);
    syslog(LOG_CRIT, "S%d thread @ sec=%6.9lf\n", threadParams->threadIdx, currentRealtime - sStartRealtime);
    printf("S%d thread @ sec=%6.9lf\n", threadParams->threadIdx, currentRealtime - sStartRealtime);

    // Check for synchronous abort request.
    while (!sAbort[threadParams->threadIdx]) {
        // wait for service request from the sequencer, a signal handler or ISR in kernel
        sem_wait(sSem + threadParams->threadIdx);
        cnt++;

        // On order of up to milliseconds of latency to get time.
        clock_gettime(MY_CLOCK_TYPE, &currentTimeVal);
        currentRealtime = realtime(&currentTimeVal);
        syslog(LOG_CRIT, "S%d 50 Hz on core %d for release %llu @ sec=%6.9lf\n", threadParams->threadIdx, 
            sched_getcpu(), cnt, currentRealtime - sStartRealtime);
    }

    // Resource shutdown here.
    pthread_exit(NULL);
}

// Received interval timer signal.
void sequencer(int id) {
    sSeqCnt++;

    // Release each service at a sub-rate of the generic sequencer rate.
    for (int i = 0; i < NUM_THREADS; ++i) {
        if ((sSeqCnt % sPeriod[i]) == 0)
            sem_post(sSem + i);
    }

    if (sAbortTest || (sSeqCnt >= sSequencePeriods)) {
        // disable interval timer
        sItime.it_interval.tv_sec = 0;
        sItime.it_interval.tv_nsec = 0;
        sItime.it_value.tv_sec = 0;
        sItime.it_value.tv_nsec = 0;
        timer_settime(sTimer1, 0, &sItime, &sLastItime);
        printf(
            "Disabling sequencer interval timer with abort=%d and %llu of %d\n", sAbortTest, sSeqCnt, sSequencePeriods);

        // Shutdown all services.
        for (int i = 0; i < NUM_THREADS; ++i) {
            sem_post(sSem + i);
            sAbort[i] = TRUE;
        }
    }
}

double getTimeMsec(void) {
    struct timespec event_ts = {0, 0};

    clock_gettime(MY_CLOCK_TYPE, &event_ts);
    return ((event_ts.tv_sec) * 1000.0) + ((event_ts.tv_nsec) / 1000000.0);
}

double realtime(struct timespec *tsptr) {
    return ((double)(tsptr->tv_sec) + (((double)tsptr->tv_nsec) / 1000000000.0));
}

void printScheduler(void) {

    int schedType = sched_getscheduler(getpid());

    switch (schedType) {
    case SCHED_FIFO:
        printf("Pthread Policy is SCHED_FIFO\n");
        break;
    case SCHED_OTHER:
        printf("Pthread Policy is SCHED_OTHER\n");
        exit(-1);
        break;
    case SCHED_RR:
        printf("Pthread Policy is SCHED_RR\n");
        exit(-1);
        break;
    default:
        printf("Pthread Policy is UNKNOWN\n");
        exit(-1);
    }
}

void main(void) {
    struct timespec currentTimeVal, currentTimeRes;
    double currentRealtime, currentRealtimeRes;

    pthread_t threads[NUM_THREADS];
    threadParams_t threadParams[NUM_THREADS];
    pthread_attr_t rtSchedAttr[NUM_THREADS];
    struct sched_param rtParam[NUM_THREADS];
    struct sched_param mainParam;
    pthread_attr_t mainAttr;

    printf("Starting High Rate Sequencer Demo\n");
    clock_gettime(MY_CLOCK_TYPE, &sStartTimeVal);
    sStartRealtime = realtime(&sStartTimeVal);
    clock_gettime(MY_CLOCK_TYPE, &currentTimeVal);
    currentRealtime = realtime(&currentTimeVal);
    clock_getres(MY_CLOCK_TYPE, &currentTimeRes);
    currentRealtimeRes = realtime(&currentTimeRes);
    printf("START High Rate Sequencer @ sec=%6.9lf with resolution %6.9lf\n", (currentRealtime - sStartRealtime),
        currentRealtimeRes);
    syslog(LOG_CRIT, "START High Rate Sequencer @ sec=%6.9lf with resolution %6.9lf\n",
        (currentRealtime - sStartRealtime), currentRealtimeRes);

    printf("System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());

    // Initialize the sequencer semaphores.
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (sem_init(sSem + i, 0, 0)) {
            printf("Failed to initialize semaphore %d\n", i);
            exit(-1);
        }
    }

    pid_t mainpid = getpid();

    int rtMaxPrio = sched_get_priority_max(SCHED_FIFO);
    int rtMinPrio = sched_get_priority_min(SCHED_FIFO);
    printf("rt_max_prio=%d\n", rtMaxPrio);
    printf("rt_min_prio=%d\n", rtMinPrio);

    // Set's the schedular of the main thread (and I assume the sequencer thread) as SCHED_FIFO and gives it
    // the highest priority.
    int rc = sched_getparam(mainpid, &mainParam);
    mainParam.sched_priority = rtMaxPrio;
    rc = sched_setscheduler(getpid(), SCHED_FIFO, &mainParam);
    if (rc < 0)
        perror("mainParam");
    printScheduler();

    int scope;
    pthread_attr_getscope(&mainAttr, &scope);
    if (scope == PTHREAD_SCOPE_SYSTEM)
        printf("PTHREAD SCOPE SYSTEM\n");
    else if (scope == PTHREAD_SCOPE_PROCESS)
        printf("PTHREAD SCOPE PROCESS\n");
    else
        printf("PTHREAD SCOPE UNKNOWN\n");

    // Set thread attributes and parameters.
    for (int i = 0; i < NUM_THREADS; i++) {
        cpu_set_t threadcpu;
        CPU_ZERO(&threadcpu);
        int cpuidx = SERVICE_CORE;
        CPU_SET(cpuidx, &threadcpu);

        pthread_attr_init(&rtSchedAttr[i]);
        pthread_attr_setinheritsched(&rtSchedAttr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&rtSchedAttr[i], SCHED_FIFO);
        pthread_attr_setaffinity_np(&rtSchedAttr[i], sizeof(cpu_set_t), &threadcpu);

        rtParam[i].sched_priority = rtMaxPrio - i - 1;
        pthread_attr_setschedparam(&rtSchedAttr[i], &rtParam[i]);

        threadParams[i].threadIdx = i;
    }

    // Create Service threads which will block awaiting release from the scheduler.
    for (int i = 0; i < NUM_THREADS; ++i) {
        rc = pthread_create(&threads[i], &rtSchedAttr[i], service, threadParams + i);
        if (rc < 0)
            perror("pthread_create for service");
        else
            printf("pthread_create successful for service %d\n", i);
    }

    // Create Sequencer thread, which like a cyclic executive, is highest priority.
    printf("Start sequencer\n");
    sSequencePeriods = 2000;

    // Sequencer = RT_MAX @ 100 Hz, set up to signal SIGALRM when timer expires.
    timer_create(CLOCK_REALTIME, NULL, &sTimer1);
    signal(SIGALRM, &sequencer);

    // Arm the interval timer.
    sItime.it_interval.tv_sec = 0;
    sItime.it_interval.tv_nsec = 10000000;
    sItime.it_value.tv_sec = 0;
    sItime.it_value.tv_nsec = 10000000;
    timer_settime(sTimer1, 0, &sItime, &sLastItime);

    // Wait for the service theads to exit.
    for (int i = 0; i < NUM_THREADS; i++) {
        if (rc = pthread_join(threads[i], NULL) < 0)
            perror("main pthread_join");
        else
            printf("joined thread %d\n", i);
    }

    printf("\nTEST COMPLETE\n");
}
