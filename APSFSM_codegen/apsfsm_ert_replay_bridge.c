#include "apsfsm_ert_replay_bridge.h"

#include <string.h>

#include "apsfsm_torque_compensation_wrapper.h"

void APSFSM_ErtReplay_step(uint8_t resetModel,
                           float speedReferencePU,
                           float speedFeedbackPU,
                           const float parameters[5],
                           const float control[3],
                           float state[4],
                           float diagnostics[4])
{
    uint32_t index;

    if (resetModel != 0u)
    {
        (void)memset(&apsfsm_torque_compensation_wrapper_DW, 0,
            sizeof(apsfsm_torque_compensation_wrapper_DW));
        (void)memset(&apsfsm_torque_compensation_wrapper_U, 0,
            sizeof(apsfsm_torque_compensation_wrapper_U));
        (void)memset(&apsfsm_torque_compensation_wrapper_Y, 0,
            sizeof(apsfsm_torque_compensation_wrapper_Y));
        apsfsm_torque_compensation_wrapper_initialize();
    }

    apsfsm_torque_compensation_wrapper_U.Speed_Reference_PU = speedReferencePU;
    apsfsm_torque_compensation_wrapper_U.Speed_Feedback_PU = speedFeedbackPU;
    for (index = 0u; index < 5u; index++)
    {
        apsfsm_torque_compensation_wrapper_U.Parameters[index] = parameters[index];
    }
    for (index = 0u; index < 3u; index++)
    {
        apsfsm_torque_compensation_wrapper_U.Control[index] = control[index];
    }

    apsfsm_torque_compensation_wrapper_step();

    state[0] = apsfsm_torque_compensation_wrapper_Y.BHat_PU;
    state[1] = apsfsm_torque_compensation_wrapper_Y.CHat_PU;
    state[2] = apsfsm_torque_compensation_wrapper_Y.Theta_rad;
    state[3] = apsfsm_torque_compensation_wrapper_Y.Covariance;
    diagnostics[0] = apsfsm_torque_compensation_wrapper_Y.Iq_Raw_PU;
    diagnostics[1] = (float)apsfsm_torque_compensation_wrapper_Y.Valid_u8;
    diagnostics[2] = (float)apsfsm_torque_compensation_wrapper_Y.Status_u8;
    diagnostics[3] = apsfsm_torque_compensation_wrapper_Y.Speed_Error_PU;
}
