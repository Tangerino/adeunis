 /*************************************************************************\
 *                                                                         *
 *  Copyright (c) 1999-2017 Carlos Tangerino (carlos.tangerino@gmail.com)  *
 *                                                                         *
 *           +--------------------------------------------+                *
 *           |     EDGE - Data Logger & Control Device    |                *
 *           |           http://tangerino.me              |                *
 *           |  WIMD.IO - IoT Data Repository & Services  |                *
 *           |             https://wimd.io                |                *
 *           +--------------------------------------------+                *
 *                                                                         *
 \**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

static int timeval_subtract(struct timeval *result, struct timeval *startTime, struct timeval *endTime) {
    /* Perform the carry for the later subtraction by updating y. */
    if (endTime->tv_usec < startTime->tv_usec) {
        int nsec = (startTime->tv_usec - endTime->tv_usec) / 1000000 + 1;
        startTime->tv_usec -= 1000000 * nsec;
        startTime->tv_sec += nsec;
    }
    if (endTime->tv_usec - startTime->tv_usec > 1000000) {
        int nsec = (endTime->tv_usec - startTime->tv_usec) / 1000000;
        startTime->tv_usec += 1000000 * nsec;
        startTime->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = endTime->tv_sec - startTime->tv_sec;
    result->tv_usec = endTime->tv_usec - startTime->tv_usec;

    /* Return 1 if result is negative. */
    return endTime->tv_sec < startTime->tv_sec;
}

void timerStart(struct timeval *ti) {
    gettimeofday(ti, NULL);
}

clock_t timerLap(struct timeval *ti) {
    struct timeval lap;
    struct timeval dif;
    gettimeofday(&lap, NULL);
    timeval_subtract(&dif, ti, &lap);
    return (clock_t) (dif.tv_sec * 1000 + (dif.tv_usec / 1000));
}

void mssleep(long ms) {
    usleep((useconds_t) ms * 1000L);
}

void dump(char *msg, uint8_t *buf, size_t size) {
    uint16_t i;
    uint16_t c;

    printf("\r\n");
    printf("%s", msg);
    printf("\r\n");
    for (c = 0, i = 0; i < size; i++) {
        char xbuf[32];
        snprintf(xbuf, sizeof (xbuf), "%02X ", buf[i]);
        printf("%s", xbuf);
        if (++c == 16) {
            printf("\r\n");
            c = 0;
        }
    }
    printf("\r\n");
}
