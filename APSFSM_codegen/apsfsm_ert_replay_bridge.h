#ifndef APSFSM_ERT_REPLAY_BRIDGE_H
#define APSFSM_ERT_REPLAY_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void APSFSM_ErtReplay_step(uint8_t resetModel,
                           float speedReferencePU,
                           float speedFeedbackPU,
                           const float parameters[5],
                           const float control[3],
                           float state[4],
                           float diagnostics[4]);

#ifdef __cplusplus
}
#endif

#endif
