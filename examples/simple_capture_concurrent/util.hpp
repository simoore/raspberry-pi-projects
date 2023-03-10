#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/ioctl.h>

/// Zeros the contents of any type.
template <typename T>
void clear(T &dest)
{
    std::memset(&dest, 0, sizeof(T));
}


/// Convience function to print an error message to std::err and then exit.
inline void errnoExit(std::string s)
{
    fprintf(stderr, "%s error %d, %s\n", s.c_str(), errno, strerror(errno));
    exit(EXIT_FAILURE);
}


inline int xioctl(int fh, int request, void *arg)
{
    int r;
    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}


inline double floatTime()
{
    struct timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    return (double)timeNow.tv_sec + (double)timeNow.tv_nsec / 1000000000.0;
}