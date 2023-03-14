#pragma once

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

class Service
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using StartRoutine = void *(*)(void *);

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    Service()
    {
        sem_init(&semExit, 0, 0);
    }

    void start(StartRoutine routine, unsigned int priority, void *args);

    /// Sleep and wait from the thread to join.
    void join(void) { pthread_join(mThread, nullptr); }

    /// Call this from main thread to tell service it should exit.
    void flagExit(void) { sem_post(&semExit); }

    /// Checks the exit semaphore to determine if it should exit.
    bool doExit();

    template <typename T>
        requires requires(T t) { t.service(); }
    static void *staticService(void *args)
    {
        reinterpret_cast<T *>(args)->service();
        return nullptr;
    }

private:
    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FIELDS
    ///////////////////////////////////////////////////////////////////////////

    pthread_t mThread;
    sem_t semExit;
};
