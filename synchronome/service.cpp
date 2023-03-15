#include <cstdio>
#include <cstdlib>

#include "service.hpp"
#include "util.hpp"


void Service::start(StartRoutine routine, unsigned int priority, void *args)
{
    pthread_attr_t pthreadAttr;
    static struct sched_param schedParam;
    pthread_attr_init(&pthreadAttr);
    pthread_attr_setinheritsched(&pthreadAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&pthreadAttr, SCHED_FIFO);
    schedParam.sched_priority = priority;
    pthread_attr_setschedparam(&pthreadAttr, &schedParam);

    int rc = pthread_create(&mThread, &pthreadAttr, routine, args);
    if (rc != 0)
    {
        perror("Failed to Make Thread\n");
        printf("return value=%d\n", rc);
        exit(EXIT_FAILURE);
    }
}


bool Service::doExit()
{
    int rc = sem_trywait(&mSemExit);
    if (rc == -1)
    {
        if (errno == EAGAIN)
        {
            return false;
        }
        else
        {
            errnoExit("Service: sem_trywait error");
        }
    }
    return true;
}
