/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : can_dtc.h
* Author        : yangming
* Date          : 2024-04-28
* Version       : 1.00
* Description   : Can dtc.
* Others        : None
*
****************************************************************************************************/
#ifndef _CAN_DTC_H
#define _CAN_DTC_H
#include "can_dtc_cfg.h"

typedef enum _dtc_appear_sts                            eDTC_APPEAR_STS;
typedef struct _dtc_snapshot                            stDTC_SNAPSHOT;
typedef struct _dtc_extended                            stDTC_EXTENDED;

enum _dtc_appear_sts
{
    DTC_DISAPPEAR = 0,
//    DTC_APPEAR_CONFIRMED,
    DTC_APPEAR_SNAPSHOT_FIRST,
    DTC_APPEAR_SNAPSHOT_LAST,
//    DTC_APPEAR_SNAPSHOT,
    DTC_APPEAR_SNAPSHOT_OVER,

    DTC_APPEAR_MAX
};

struct _dtc_snapshot
{
    // uint16              vehicle_spd;
    // uint16              hv;
    // uint08              lv;
    uint08              runCmd;
    uint16              spdCmd;
    uint16              Max_power_acc;
    uint08              BusCurrent;
    uint08              PhaseCurrent;
    uint08              Temp_COM_SIC;
    uint16              speed_actal;
    uint16              fault_status_flag_motor;
    uint32              run_time;
    uint16              pVdcValue;
    uint08              Temp_PCB;
    uint08              record_number;
};
struct _dtc_extended
{
    uint32              fault_occ_cnt;
    uint32              fault_pending_cnt;
    uint08              aging_cnt;
    uint08              aged_cnt;
};
extern stDTC_SNAPSHOT g_dtc_snapshot[CAN_DTC_NUM]; /* write to eeprom */
extern stDTC_SNAPSHOT g_dtc_local_snapshot[CAN_DTC_NUM]; /* write to eeprom */
extern uint08 g_dtc_confirmed_sts[CAN_DTC_NUM]; /* write to eeprom */
extern stDTC_EXTENDED  g_dtc_extended[CAN_DTC_NUM]; /* write to eeprom */
void can_dtc_init(void);
void can_dtc_process(void);
extern uint08 dtc_operation_cycle_switch_get(uint08 sw);
extern void dtc_operation_cycle_switch_set(uint08 sw);
extern void dtc_clear(eDTC_TYP typ);
extern void dtc_all_clear(void);
extern void dtc_cur_snapshot_get(eDTC_TYP typ, stDTC_SNAPSHOT shnapshot);
extern uint08 dtc_sts_get(eDTC_TYP typ);
extern uint16 dtc_number_get(void);
extern uint16 dtc_number_get_by_sts(uint08 sts);
extern typ_bool is_dtc_snapshot_recorded(eDTC_TYP typ, uint08 record_number);
extern void dtc_shapshot_global_data_get(eDTC_TYP typ, stDTC_SNAPSHOT* snapshot_data);
extern void dtc_shapshot_local_data_get(eDTC_TYP typ, stDTC_SNAPSHOT* snapshot_data);
extern void dtc_ignon_condition_set(uint08 val);
extern uint08 dtc_ignon_condition_get(void);

#endif /* _CAN_DTC_H */
