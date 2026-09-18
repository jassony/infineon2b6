#ifndef XCP_CAL_TEST_H
#define XCP_CAL_TEST_H

#include <stdint.h>

#define XCP_CAL_M0_RAM_ADDRESS                 (0x08000800u)
#define XCP_CAL_M0_RAM_SIZE                    (0x00000100u)
#define XCP_CAL_M4_RAM_ADDRESS                 (0x0800D000u)
#define XCP_CAL_M4_RAM_SIZE                    (0x00000100u)

/* Add future M0 calibration fields here. Keep this structure within 256 bytes. */
typedef struct
{
    uint16_t testValue;
    uint16_t motorSpeedRpm;
    uint16_t powerLimit;
    uint16_t accelerationTime;
    uint16_t decelerationTime;
    uint8_t motorEnable;
    uint8_t applySequence;
    uint8_t appliedSequence;
    uint8_t reserved0;
    uint16_t reserved1;
} XcpM0CalibrationType;

extern volatile XcpM0CalibrationType XcpM0Calibration;

#define XcpCalTestValue (XcpM0Calibration.testValue)

void XcpCalM0Apply(void);

#endif
