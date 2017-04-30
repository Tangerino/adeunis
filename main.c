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

/*
 MIT License

Copyright (c) 2017 Carlos Tangerino

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
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

