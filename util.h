

/* 
 * File:   util.h
 * Author: Carlos Tangerino <carlos.tangerino@gmail.com>
 *
 * Created on April 30, 2017, 8:17 AM
 */

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
    
    void timerStart(struct timeval *ti);
    clock_t timerLap(struct timeval *ti);
    void mssleep(long ms);
    void dump(char *msg, uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */

