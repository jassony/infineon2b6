/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : module_header_file.h
* Author        : yangming
* Date          : 2024-05-24
* Version       : 1.00
* Description   : all module header files
* Others        : None
*
*******************************************************************************************************/
#ifndef _MODULE_HEADER_FILE_H
#define _MODULE_HEADER_FILE_H

#include "common_mem_op.h"
#include "performance_test.h"
#ifdef IN_M0_BOOT
#include "ringbuf.h"
#endif
#include "soft_timer.h"

#ifndef USE_M4
#include "can_dcm.h"
#include "can_user.h"
#include "dtc_user.h"
#include "bootloader.h"
#include "uds_user.h"
#include "eeprom_app.h"

#endif
//#include "input.h"
//#include "output.h"
//#include "adc.h"
//#include "pwm_ctrl.h"
//#include "j1939_dcm.h"
//#include "j1939_multiframe.h"
//#include "peripheral_include.h"
//#include "lin_user.h"
//#include "power_manage.h"
//#include "cmos_74hc4051.h"

#endif /* _MODULE_HEADER_FILE_H */
