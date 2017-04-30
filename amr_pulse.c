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
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"

#define GLOBAL_TIMEOUT 1000 // in milliseconds

#include "amr_pulse.h"

static size_t amrReadBytes(int fd, unsigned char *buffer, size_t bytes) {
    assert(buffer);
    if (bytes == 0) {
        return 0;
    }
    unsigned char c;
    struct timeval timer;
    timerStart(&timer);
    size_t byteIndex = 0;
    do {
        ssize_t sread = read(fd, &c, 1);
        if (sread > 0) {
            buffer[byteIndex] = c;
            byteIndex++;
            if (byteIndex == bytes) {
                break;
            }
        } else {
            mssleep(10); // don't kill the CPU
        }
    } while (timerLap(&timer) < GLOBAL_TIMEOUT);
    return byteIndex;
}

static int startOfFrame(int fd) {
    int status = AMR_ERR_TIMEOUT;
    unsigned char c;

    size_t bytes = amrReadBytes(fd, &c, 1);
    if (bytes == 1) {
        if (c == START_OF_FRAME) {
            status = AMR_ERR_OK;
        }
    }
    return status;
}

static size_t amrReceiveFrame(int fd, unsigned char *buffer, size_t bufferLen) {
    for (;;) {
        int ok = startOfFrame(fd);
        if (ok == AMR_ERR_OK) {
            break;
        }
    }
    uint8_t controlField[12]; // + rrsi field 11 + 1
    size_t bytes = amrReadBytes(fd, controlField, sizeof (controlField));
    if (bytes != sizeof (controlField)) {
        dump("Timeout reading control field", controlField, bytes);
        return 0;
    }
    size_t dataFrameSize = (size_t) (controlField[0]);
    if (dataFrameSize == 0) {
        dump("Weird data frame size", controlField, bytes);
        return 0;
    }
    size_t frameSize = dataFrameSize + sizeof (controlField); // the total frame control + data
    if (frameSize > bufferLen) {
        printf("Received frame too big %d of %d", (int) frameSize, (int) bufferLen);
        return 0;
    }
    memcpy(buffer, controlField, sizeof (controlField));

    bytes = amrReadBytes(fd, buffer + sizeof (controlField), dataFrameSize);
    if (bytes != dataFrameSize) {
        dump("Some data is missing", buffer, bytes);
        return 0;
    }
    return bytes + dataFrameSize;
}

static int extractDataFrame(CAMRDataFrame *amr, unsigned char *buffer) {
    if (buffer[1] != 0x44) {
        return AMR_INVALID_CONTROL_FIELD;
    }
    amr->c.control = buffer[1];
    amr->m.manufacturerId = (uint16_t) (buffer[3] << 8) + buffer[2];

    amr->meterNumber = (buffer[7] & 0xf0) >> 4;

    amr->a.serialNumber =
            (uint32_t) (buffer[7] << 24) +
            (uint32_t) (buffer[6] << 16) +
            (uint32_t) (buffer[5] << 8) +
            (uint32_t) buffer[4];

    amr->a.version = buffer[8];
    amr->a.deviceType = buffer[9];

    amr->ci.frameCode = buffer[10];

    amr->d.accessNumber = buffer[11];
    amr->d.status = buffer[12];
    amr->d.signature = (uint16_t) (buffer[13] << 8) + buffer[14];
    amr->d.encryptionVerification = (uint16_t) (buffer[15] << 8) + buffer[16];
    amr->d.dataInformationField = buffer[17];
    amr->d.valueInformationField = buffer[18];

    amr->d.data =
            (uint32_t) (buffer[22] << 24) +
            (uint32_t) (buffer[21] << 16) +
            (uint32_t) (buffer[20] << 8) +
            (uint32_t) buffer[19];
    amr->rssi.value = (int) (-125.0 + ((float) buffer[23] * 0.5));
    return AMR_ERR_OK;
}

static int extractHistoricalFrame(CAMRDataFrame *amr, unsigned char *buffer) {
    return AMR_ERR_OK;
}

static int extractFrameFields(CAMRDataFrame *amr, unsigned char *buffer) {
    amr->l.frameSize = buffer[0];
    if (amr->l.frameSize == 0x0c) {
        return extractDataFrame(amr, buffer);
    } else if (amr->l.frameSize == 0x0c) {
        return extractHistoricalFrame(amr, buffer);
    } else {
        return AMR_ERR_UNKNOWN_FRAME;
    }
}

static void dumpAmrFrame(CAMRDataFrame *amr) {
    time_t current_time;
    char *c_time_string;
    current_time = time(NULL);
    c_time_string = ctime(&current_time);
    printf("\n---- %s", c_time_string);
    printf("Meter: %d\n", (int) amr->meterNumber);
    printf("Serial number: %04lx%lx%02lx%02lx\n", 
            (unsigned long)amr->m.manufacturerId, 
            amr->a.serialNumber,
            (unsigned long)amr->a.version,
            (unsigned long)amr->a.deviceType);
    const char *deviceType = "?";
    switch (amr->a.deviceType) {
        case 0x03:
            deviceType = "Gaz meter";
            printf("%s: %g\n", deviceType, (float)amr->d.data);
            break;
        case 0x07:
            deviceType = "Water meter";
            printf("%s: %gL (%gm3)\n", deviceType, amr->d.data / 10.0, amr->d.data / 10000.0);
            break;
        case 0x02:
            deviceType = "Electricity meter";
            printf("%s: %g\n", deviceType, (float)amr->d.data);
            break;
        case 0x0c:
            deviceType = "Heat Meter";
            printf("%s: %g\n", deviceType, (float)amr->d.data);
            break;
        default:
            deviceType = "Unknown meter";
            printf("%s: %g\n", deviceType, (float)amr->d.data);
            break;
    }
    
    const char *status = "";
    switch (amr->d.status) {
        case 0b00000000:
            status = "OK";
            break;
        case 0b0000100:
            status = "Low battery";
            break;
        case 0b00110010:
            status = "Con g switch error";
            break;
        case 0b10010010:
            status = "Temporary fraud";
            break;
        case 0b10001010:
            status = "Permanent fraud";
            break;
        case 0b10010110:
            status = "Temporary fraud + Low battery";
            break;
        default:
            status = "Unknown";
            break;
            
    }
    printf ("Status %s (%lx)\n", status, (unsigned long)amr->d.status);
    printf("RSSI: %d dBm\n", amr->rssi.value);
}

void amrDecode(int fd) {
    unsigned char buffer[256];
    size_t bytes = amrReceiveFrame(fd, buffer, sizeof (buffer));
    if (bytes == 0) {
        return;
    }
    dump ("New frame", buffer, bytes);
    CAMRDataFrame amr;
    int status = extractFrameFields(&amr, buffer);
    if (status != AMR_ERR_OK) {
        return;
    }
    dumpAmrFrame(&amr);
}