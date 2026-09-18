/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : software_version.h
* Author        : yangming
* Date          : 2024-06-13
* Version       : 1.00
* Description   : softerware version manage.
* Others        : None
*
*******************************************************************************************************/
#ifndef _SOFTWARE_VERSION_H
#define _SOFTWARE_VERSION_H
#include "platform_common_typdef.h"
//#include "def_app.h"

/* The software version is designed as follows:
   version high byte(hex) + version middle byte(hex) + version low byte(hex);
   e.g. 0x010001, means V01.00.01.

   The software build date includes year,month and day,where the year is only 1 byte;
   e.g. 0x240613, means June 13,2024.
*/
#define LOCAL_SW_VER_H                      0x01
#define LOCAL_SW_VER_M                      0x01
#define LOCAL_SW_VER_L                      0x02
#define LOCAL_SW_VER_MINI                   0x01

#define LOCAL_SW_M4_VER_H                   0x02
#define LOCAL_SW_M4_VER_M                   0x02
#define LOCAL_SW_M4_VER_L                   0x02
#define LOCAL_SW_M4_VER_MINI                0x01


#define LOCAL_HW_VER_H                      0x01
#define LOCAL_HW_VER_M                      0x05
#define LOCAL_HW_VER_L                      0x00

#define LOCAL_BUILD_DATE_YEAR               0x25
#define LOCAL_BUILD_DATE_MONTH              0x12
#define LOCAL_BUILD_DATE_DAY                0x24

#define LOCAL_BOOT_VER_H                    0x01
#define LOCAL_BOOT_VER_M                    0x00
#define LOCAL_BOOT_VER_L                    0x09

#define LOCAL_DEF_VAL                       0xAA
#define LOCAL_SPC_VAL                       0x55

#define USER_INFO_SIZE                      0x20

#endif /* _SOFTWARE_VERSION_H */

