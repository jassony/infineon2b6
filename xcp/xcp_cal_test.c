#include "xcp_cal_test.h"
#include "m0_var.h"

#if defined(__ICCARM__)
#pragma location = ".xcp_cal_m0"
__root volatile XcpM0CalibrationType XcpM0Calibration =
{
    0x1234u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u
};
#else
volatile XcpM0CalibrationType XcpM0Calibration =
{
    0x1234u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u
};
#endif

void XcpCalM0Apply(void)
{
    static uint8_t lastApplySequence;
    const uint8_t requestSequence = XcpM0Calibration.applySequence;

    if (requestSequence == lastApplySequence)
    {
        return;
    }

    Motor_Control_Set(XcpM0Calibration.motorEnable,
                      XcpM0Calibration.motorSpeedRpm,
                      XcpM0Calibration.powerLimit);
    Acceleration_Time_Set(XcpM0Calibration.accelerationTime);
    Reduction_Time_Set(XcpM0Calibration.decelerationTime);

    XcpM0Calibration.appliedSequence = requestSequence;
    lastApplySequence = requestSequence;
}
