/*
 * m0_canfd_cfg.h
 *
 *  Created on: 2026Äê1ÔÂ22ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_CANFD_INC_M0_CANFD_CFG_H_
#define M0_BSW_SOURCE_LD_CANFD_INC_M0_CANFD_CFG_H_
#include "cy_project.h"

#define TX_BUFFER_NUM 32

void Canfd_Init(void);
void Can_Transmit(cy_pstc_canfd_type_t chn, uint32_t canid, uint8_t dlc, uint8_t * data,bool fd);
void Canfd_Deinit(void);


#endif /* M0_BSW_SOURCE_LD_CANFD_INC_M0_CANFD_CFG_H_ */
