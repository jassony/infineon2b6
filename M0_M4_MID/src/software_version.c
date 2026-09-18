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
Version: 01.00.08.00
 Date: 2025-07-22
 Description: 
 1、app修改tp层(参考A12RE)

Version: 01.00.07.01
 Date: 2025-05-16
 Description: 
 1、app修改DTC故障逻辑

Version: 01.00.07.00
 Date: 2025-05-15
 Description: 
 1、修改tp层，解决偶发控制器未回复连续帧问题
 2、修改S3Server时间(2.5s改为5s)，由于调用了2次计时函数导致
 3、85服务正常响应但是没有实际作用问题修改(未调用禁止DTC函数)
 4、电压采集不及时导致诊断延迟的问题修改，100ms调用周期导致，改为上电先采集一次，然后电压判定从100ms改为150ms

Version: 01.00.07.00
 Date: 2025-04-10
 Description: 
 1、修改tp层发送功能寻址问题
 2、增加自检功能，31服务配合
 
Version: 01.00.06.03
 Date: 2025-03-06
 Description: 
 1、修改busoff处理
 2、修改CAN发送CANIF_HTH1_TMC_BSW_VERSION赋值错误问题
 
 Version: 01.00.06.02
 Date: 2025-03-04
 Description: 
 1、s_ee_wr_temp_buf数组定义空间大小为EE_WR_ITEM_TOTAL_SIZE，
    否则可能造成数组溢出（eeprom_item_write_chk中调用导致）
 2、扩大EE_WR_BUF_NUM和EE_WR_ITEM_BUF_SIZE，对应DTC较多
 3、修改结构体_service_22_ctrl下的成员,删除did值，放到全局变量中，以免造成对齐问题
 4、增加部分DID
 5、取消PTC3的失效判定
 6、增加TMSCAN上BSW版本外发的小版本号MINI
 7、修改DTC存储机制，在DTC变化的时候更新（500ms延时）；同时取消本地快照
 8、增加2F的DID信息（实际不处理，只回复）
 9、修改种子随机数
 
 
 Version: 01.00.06.01
 Date: 2025-02-26
 Description: 
 1、增加DTC和DID功能。
 
 Version: 01.00.06.00
 Date: 2025-02-25
 Description: 
 1、基于3AA0的UDS和JH6的基础功能进行移植整合。
*/

