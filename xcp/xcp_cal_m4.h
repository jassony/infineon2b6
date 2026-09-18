#ifndef XCP_CAL_M4_H
#define XCP_CAL_M4_H

#include <stdint.h>

/* Add future M4 calibration fields here. Keep this structure within 256 bytes. */
typedef struct
{
    uint16_t forceDutyU;
    uint16_t forceDutyV;
    uint16_t forceDutyW;
    uint8_t forceDutyEnable;
    uint8_t applySequence;
    uint8_t appliedSequence;
    uint8_t reserved0;
    uint16_t reserved1;
} XcpM4CalibrationType;

extern volatile XcpM4CalibrationType XcpM4Calibration;

#endif
