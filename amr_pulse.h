
/* 
 * File:   amr_decode.h
 * Author: Carlos Tangerino <carlos.tangerino@gmail.com>
 *
 * Created on April 30, 2017, 7:44 AM
 */

#ifndef AMR_DECODE_H
#define AMR_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    enum {
        AMR_ERR_OK = 0,
        AMR_ERR_TIMEOUT,
        AMR_ERR_UNKNOWN_FRAME,
        AMR_INVALID_CONTROL_FIELD
    };

#define START_OF_FRAME (0xff)
#define FRAME_SIZE_DATA (0x0c)
#define FRAME_SIZE_HIST (0x19)

    typedef struct {
        uint8_t frameSize;
    } CLField;

    typedef struct {
        uint8_t control;
    } CCField;

    typedef struct {
        uint16_t manufacturerId;
    } CMField;

    typedef struct {
        unsigned long serialNumber;
        uint8_t version;
        uint8_t deviceType;
    } CAField;

    typedef struct {
        uint8_t frameCode;
    } CCIField;

    typedef struct {
        uint8_t accessNumber;
        uint8_t status;
        uint16_t signature;
        uint16_t encryptionVerification;
        uint8_t dataInformationField;
        uint8_t valueInformationField;
        int32_t data;
    } CDataField;

    typedef struct {
        int value;
    } CRSSI;

    typedef struct {
        uint8_t meterNumber;
        CLField l;
        CCField c;
        CMField m;
        CAField a;
        CCIField ci;
        CDataField d;
        CRSSI rssi;
    } CAMRDataFrame;

    void amrDecode(int(fd));


#ifdef __cplusplus
}
#endif

#endif /* AMR_DECODE_H */

