#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

//#define MAX (1000000ULL)                       // primes between 0 and 1 million
//#define MAX (10000000ULL)                      // primes between 0 and 10 million
#define MAX (100000000ULL)                   // primes between 0 and 100 million
//#define MAX (1000000000ULL)                  // primes between 0 and 1 billion
//#define MAX (0xFFFFFFFFULL)                  // primes between 0 and 2^32-1
//#define MAX (0xFFFFFFFFFFFFFFFFULL)          // primes between 0 and 2^64-1

#define FLOAT double

unsigned char isprime[MAX+1];
unsigned int value[MAX+1];

int main(void)
{
    int i, j;
    unsigned int p=2, cnt=0;
    struct timespec start, now;
    FLOAT fstart=0.0, fnow=0.0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (FLOAT)start.tv_sec  + (FLOAT)start.tv_nsec / 1000000000.0;
    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (FLOAT)now.tv_sec  + (FLOAT)now.tv_nsec / 1000000000.0;
    printf("\nstart test at %lf\n", fnow-fstart);


    // not prime by definition
    isprime[0]=0; value[0]=0;
    isprime[1]=0; value[1]=1;

    for(i=2; i<MAX+1; i++) { isprime[i]=1; value[i]=i; }

    while( (p*p) <=  MAX)
    {
        // invalidate all multiples of lowest prime so far
        for(j=2*p; j<MAX+1; j+=p) isprime[j]=0;

        // find next lowest prime
        for(j=p+1; j<MAX+1; j++) { if(isprime[j]) { p=j; break; } }
    }


    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (FLOAT)now.tv_sec  + (FLOAT)now.tv_nsec / 1000000000.0;
    printf("\nstop test at %lf\n", fnow-fstart);

    for(i=0; i<MAX+1; i++) { if(isprime[i]) { cnt++; } }
    printf("\nTested %u numbers per second and the number of primes [0..%llu]=%u\n\n", (unsigned int) (MAX / (fnow-fstart)), MAX, cnt);
}
