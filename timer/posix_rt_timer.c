#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NSEC_PER_MICROSEC 1000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_SEC 1000000000

#define OK (0)
#define ERROR (-1)

// basic tick default in milliseconds
int period_millisecs = 10;
unsigned long long period_nsecs = 10000000;

static timer_t tt_timer;
static struct itimerspec last_itime;
static struct itimerspec itime = {{0, NSEC_PER_SEC}, {0, NSEC_PER_SEC}};

static unsigned long tick_count = 0;
static struct timespec rtclk_time = {0, 0};
static struct timespec rtclk_start_time = {0, 0};
static struct timespec rtclk_last_time = {0, 0};
static unsigned long accum_jitter_nsec = 0;
static int tick_error = 0;

void ptimer_shutdown(void);
void monitor_interval_expired(int signo, siginfo_t *info, void *ignored);
void intHandler(int dummy) {
    ptimer_shutdown();
}

int main(int argc, char *argv[]) {
    int i, my_tid = 0, ticks_per_sec;
    int flags = 0;
    struct timespec rtclk_resolution;

    struct sigevent ReleaseEvent;
    struct sigaction ReleaseAction;

    if (argc == 2) {
        period_millisecs = atoi(argv[1]);
    } else if (argc == 1) {
        printf("Using period of %d milliseconds\n", period_millisecs);
    } else {
        fprintf(stderr, "Usage: %s <period-millisecs>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    period_nsecs = period_millisecs * NSEC_PER_MSEC;

    // get thread id here
    // my_tid = taskIdSelf();

    // Handle Ctrl-C termination request
    signal(SIGINT, intHandler);

    ReleaseEvent.sigev_notify = SIGEV_SIGNAL;
    ReleaseEvent.sigev_signo = SIGRTMIN + 1;
    ReleaseEvent.sigev_value.sival_int = my_tid;

    ReleaseAction.sa_sigaction = monitor_interval_expired;
    ReleaseAction.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN + 1, &ReleaseAction, NULL);

    tick_count = 0;
    rtclk_time.tv_sec = 0;
    rtclk_time.tv_nsec = 0;

    accum_jitter_nsec = 0;

    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = (period_millisecs * NSEC_PER_MSEC);
    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = (period_millisecs * NSEC_PER_MSEC);

    if (clock_getres(CLOCK_REALTIME, &rtclk_resolution) == ERROR)
        perror("clock_getres");

    // ticks_per_sec = sysClkRateGet();
    //  Need equivalent fetch of the Linux software timer services resolution (tick)
    ticks_per_sec = sysconf(_SC_CLK_TCK);

    printf(
        "\n\nClock demo using system RT clock with resolution:\n\t%ld secs, %ld microsecs (%ld nanosecs)\n\t%ld LINUX "
        "ticks/sec; %ld nsecs/tick\n",
        rtclk_resolution.tv_sec, (rtclk_resolution.tv_nsec / 1000), rtclk_resolution.tv_nsec, ticks_per_sec,
        (period_millisecs * NSEC_PER_MSEC));

    // if(taskPrioritySet(my_tid, 1) == ERROR)
    //   perror("priority set");
    // else {
    //   printf("Task priority set to 0\n");
    //   fflush(stdout);
    // }

    /* set up to signal if timer expires */
    timer_create(CLOCK_REALTIME, NULL, &tt_timer);

    /* initialize internal timer release wrapper */
    timer_create(CLOCK_REALTIME, &ReleaseEvent, &tt_timer);

    timer_settime(tt_timer, flags, &itime, &last_itime);

    clock_gettime(CLOCK_REALTIME, &rtclk_start_time);
    rtclk_last_time.tv_sec = rtclk_start_time.tv_sec;
    rtclk_last_time.tv_nsec = rtclk_start_time.tv_nsec;

    /* keep program memory resident */
    while (1) {
        pause();
    }
}

void monitor_interval_expired(int signo, siginfo_t *info, void *ignored) {
    int prio;
    int mon_tid = info->si_value.sival_int;

    // get priority of thread handling and print it
    // taskPriorityGet(taskIdSelf(), &prio);
    // printf("handler task prio=%d\n", prio);
    // fflush(stdout);

    tick_count++;
    clock_gettime(CLOCK_REALTIME, &rtclk_time);

    printf("Time = %ld secs, %ld nsecs, Prev time = %ld secs, %ld nsecs\n", rtclk_time.tv_sec, rtclk_time.tv_nsec,
        rtclk_last_time.tv_sec, rtclk_last_time.tv_nsec);
    printf("Delta-time = %ld secs, %ld nsecs\n", (rtclk_time.tv_sec - rtclk_last_time.tv_sec),
        (rtclk_time.tv_nsec - rtclk_last_time.tv_nsec));
    fflush(stdout);

    tick_error = (((rtclk_time.tv_sec - rtclk_last_time.tv_sec) * NSEC_PER_SEC) +
                     (rtclk_time.tv_nsec - rtclk_last_time.tv_nsec)) -
                 period_nsecs;
    accum_jitter_nsec += (unsigned long)abs(tick_error);

    rtclk_last_time.tv_sec = rtclk_time.tv_sec;
    rtclk_last_time.tv_nsec = rtclk_time.tv_nsec;
}

void ptimer_shutdown(void) {
    int status;

    /* disable and delete timer */

    // Cancel is now built into delete
    // if((status = timer_cancel(tt_timer)) == ERROR)
    //  perror("tt_timer");

    if ((status = timer_delete(tt_timer)) == ERROR)
        perror("tt_timer");
    else {
        printf("\nTick/Tock interval timer deleted, status=%d\n", status);
        fflush(stdout);
    }

    printf("RT clock start seconds = %ld, nanoseconds = %ld\n", rtclk_start_time.tv_sec, rtclk_start_time.tv_nsec);

    printf("RT clock end seconds = %ld, nanoseconds = %ld\n", rtclk_time.tv_sec, rtclk_time.tv_nsec);

    printf("RT clock delta seconds = %ld, nanoseconds = %ld\n", (rtclk_time.tv_sec - rtclk_start_time.tv_sec),
        (rtclk_time.tv_nsec - rtclk_start_time.tv_nsec));

    printf(
        "Timer tick_count = %ld @ %d millisecs/tick, %lu nanosecs/tick\n", tick_count, period_millisecs, period_nsecs);
    printf("Tick-based delta = %ld secs, %ld nsecs\n", (tick_count / period_millisecs),
        (tick_count % period_millisecs) * period_nsecs);

    printf("Accumulated nanoseconds of jitter = %ld\n", accum_jitter_nsec);

    exit(0);
}
