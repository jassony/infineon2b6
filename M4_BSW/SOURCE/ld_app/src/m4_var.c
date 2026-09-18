/*
 * m4_var.c
 *
 *  Created on: 2026��2��24��
 *      Author: hzldy
 */

#include "m4_var.h"
#include "m0_m4_ipc.h"



uint16_t bus_voltage;//��to m0)
uint16_t phase_current;//��to m0)
uint16_t motorspeedfeedbackq15;//��to m0)
uint16_t bus_current;//��to m0)
//uint16_t err_status;//��to m0)
//uint16_t running_status;//��to m0)
uint16_t g_ad_vbat_sense;//ĸ�ߵ�ѹ��to m0)
//uint16_t expect_speed;//
//uint8_t  power_status;//(���ػ���
uint16_t acctime;
uint16_t redtime;

uint32_t motor_err_status; //(�������״̬��Ϣ)
uint16_t motor_err_Clear;//(����������)
uint16_t motor_running_status;//�����״̬���������������ػ������ϵȣ�
uint8_t motor_enable_command;
uint16_t motorspeedreferenceq10;
uint16_t motor_power_limiter;//��������ƣ�

uint32_t g_m0_fault_status = 0;