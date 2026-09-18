/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : software_version.c
* Author        : yangming
* Date          : 2024-06-13
* Version       : 1.00
* Description   : softerware version manage.
* Others        : None
*
*******************************************************************************************************/

/*

  Version: 01.00.08
 Date: 2025-07-22
 Description:
 1、修改tp层(参考A12RE)

 Version: 01.00.07
 Date: 2025-05-15
 Description:
 1、同步app修改(app版本 : 01.00.07)
 2、在10 02切换到boot下，且未进行app升级的时候，不判断下载失效时间

 Version: 01.00.06
 Date: 2025-04-10
 Description:
 1、修改bootloader的擦除流程: 在擦除之前先擦除app有效标志的地址，防止在擦除过程中意外退出但是app有效标志还是有效状态

 Version: 01.00.05
 Date: 2025-03-08
 Description:
 1、TP层修改：修改在多包接收过程中，功能寻址和物理寻址的SF对多包接收的影响，导致接收失败的问题

 Version: 01.00.04
 Date: 2025-03-04
 Description:
 1、增加F184读写；
 2、修改结构体_service_22_ctrl下的成员,删除did值，放到全局变量中，以免造成对齐问题
 
 Version: 01.00.03
 Date: 2025-02-26
 Description: 
 1、bootloader_main_process中增加状态机BL_STEP_WAIT_FOR_REST，用于在下载完成，并且校验成功后，等待11 01命令。
 2、增加pending的超时判定，超时3s后自动重启。
 
 Version: 01.00.02
 Date: 2025-02-24
 Description: 
 1、从3AA0移植。
*/

