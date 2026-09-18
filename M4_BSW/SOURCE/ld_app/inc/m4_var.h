/*
 * m4_var.h
 *
 *  Created on: 2026��2��24��
 *      Author: hzldy
 */

#ifndef M4_BSW_SOURCE_LD_APP_INC_M4_VAR_H_
#define M4_BSW_SOURCE_LD_APP_INC_M4_VAR_H_

#include "cy_project.h"
#include "cy_device_headers.h"

extern uint8_t g_pwm_thick_film;//��Ĥpwmռ�ձ�0-100




extern uint16_t bus_voltage;//��to m0)
extern uint16_t phase_current;//��to m0)
extern uint16_t motorspeedfeedbackq15;//��to m0)
extern uint16_t bus_current;//��to m0)
//extern uint16_t err_status;//��to m0)
//extern uint16_t running_status;//��to m0)
extern uint16_t g_ad_vbat_sense;//ĸ�ߵ�ѹ��to m0)
//extern uint16_t expect_speed;//
//extern uint8_t  power_status;//(���ػ���
extern uint16_t acctime;
extern uint16_t redtime;

extern uint32_t motor_err_status; //(�������״̬��Ϣ)
extern uint16_t motor_err_Clear;//(����������)
extern uint16_t motor_running_status;//�����״̬���������������ػ������ϵȣ�
extern uint8_t motor_enable_command;
extern uint16_t motorspeedreferenceq10;
extern uint16_t motor_power_limiter;//��������ƣ�

extern uint32_t g_m0_fault_status;
#endif /* M4_BSW_SOURCE_LD_APP_INC_M4_VAR_H_ */
