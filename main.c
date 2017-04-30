
/* 
 * File:   main.c
 * Author: Carlos Tangerino <carlos.tangerino@gmail.com>
 *
 * Created on April 30, 2017, 7:33 AM
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <termios.h>

#include "amr_pulse.h"

static int openSerialPort(const char *device, unsigned int baud) {
    int fd;
    struct termios newtio;
    memset(&newtio, 0, sizeof (newtio));
    if ((fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
        return -1;
    } else {
        // set new port settings for canonical input processing
        newtio.c_cflag = baud | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;
        newtio.c_lflag &= ~(ICANON);
        newtio.c_lflag &= ~(ECHO);
        newtio.c_lflag &= ~(ECHOE);
        newtio.c_cc[VMIN] = 1;
        newtio.c_cc[VTIME] = 1;
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);
    }
    return fd;
}

/*
 * 
 */
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: amrpulse [serial port]\n");
        printf("Example: amrpulse /dev/ttyACM0\n");
        return (EXIT_FAILURE);
    }
    int fd = openSerialPort(argv[1], B115200);
    if (fd < 0) {
        printf("Open serial port error (%s)\n", argv[1]);
        return (EXIT_FAILURE);
    }
    for (;;) {
        amrDecode(fd);
    }
    close(fd);
    return (EXIT_SUCCESS);
}

