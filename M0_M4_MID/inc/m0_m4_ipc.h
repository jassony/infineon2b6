/*
 * m0_m4_ipc.h
 *
 *  Created on: 2026年2月6日
 *      Author: hzldy
 */

#ifndef M0_M4_MID_INC_M0_M4_IPC_H_
#define M0_M4_MID_INC_M0_M4_IPC_H_
//#include "user_funtion.h"
#include "cy_project.h"
#ifdef USE_M4


#include "m4_ipc_cfg.h"

#elif defined(USE_M0)

#include "m0_ipc_cfg.h"

#endif

typedef union 
{
  struct
  {
    uint32_t value:31;
    uint32_t receive_flag:1;

  };
  uint32_t data;
}_UN_SET_DATA;

typedef struct
{
    uint32_t clientId;  /* PIPE I/F internal use area. Must be first element of the structure. */
    _UN_SET_DATA bus_voltage;
    _UN_SET_DATA phase_current;
    _UN_SET_DATA motor_speed;
    _UN_SET_DATA bus_current;
    _UN_SET_DATA err_status;
    _UN_SET_DATA running_status;
    _UN_SET_DATA phase_current_u;      
    _UN_SET_DATA phase_current_v;       
    _UN_SET_DATA phase_current_w; 
    _UN_SET_DATA motor_power;
    _UN_SET_DATA ipm_fault_state;

    /* ---- DEBUG: packed dq Q15 [Vd:15-0|Vq:31-16], [Id:15-0|Iq:31-16] ---- */
    _UN_SET_DATA debug_VdVq;         /* Vd_q15 bits[15:0], Vq_q15 bits[31:16] */
    _UN_SET_DATA debug_IdIq;         /* Id_q15 bits[15:0], Iq_q15 bits[31:16] */

    /* ---- DEBUG: modulation ratio & voltage utilization ----
     * bits[15:0]  = mod_ratio_q15      (|Vref|/Vdc, Q15; linear SVPWM ceiling 2/√3≈1.1547)
     * bits[29:16] = v_util_pct×100     (0.01%/LSB; 100% = linear-modulation limit)
     * bit30       = overmodulation flag
     * bit31       = receive_flag                                   */
    _UN_SET_DATA debug_mod;

//    _UN_SET_DATA g_ad_vbat_sense;
    uint32_t checksum;

} ST_M0_GET_M4_SET_DATA;

typedef union 
{
  struct
  {
    uint16_t enable_command:1;
    uint16_t speed:15;
    uint16_t power_limiter:15;
    uint32_t receive_flag:1;


  };
  uint32_t data;
}_UN_MOTOR_CTR;

typedef struct
{
    uint32_t clientId;  /* PIPE I/F internal use area. Must be first element of the structure. */
//    uint32_t expect_speed;
//    uint32_t power_status;
    _UN_MOTOR_CTR motor_control;
    _UN_SET_DATA err_clr;
    _UN_SET_DATA acctime;
    _UN_SET_DATA redtime;
    _UN_SET_DATA Word;
    uint32_t checksum;

} ST_M4_GET_M0_SET_DATA;


#ifdef USE_M4
extern ST_M0_GET_M4_SET_DATA m4_set_data;
extern ST_M4_GET_M0_SET_DATA m4_get_data;
#elif defined(USE_M0)

extern ST_M0_GET_M4_SET_DATA m0_get_data;
extern ST_M4_GET_M0_SET_DATA m0_set_data;
#endif
void Ipc_Pipe_Set(void);
void Ipc_Init_Pipe(void);
uint32_t IPC_Checksum_Count(uint32_t *ptr,uint16_t len);

#endif /* M0_M4_MID_INC_M0_M4_IPC_H_ */
