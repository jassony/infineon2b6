/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : bootloader.c
* Author        : yangming
* Date          : 2024-03-26
* Version       : 1.00
* Description   : UDS bootloader.
* Others        : None
*
****************************************************************************************************/
#include "bootloader.h"
#include "soft_timer.h"
#include "eeprom_driver.h"
#include "m0_flash.h"
//#include "Cpu.h"
//#include "system_S32K144.h"
//#include "wdog_user.h"

#define BL_WDG_FEED
#define WR_UNIT_BYTE_NUM                                DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH
#define BL_WR_BUF_SIZE                                  (DCM_RX_BUF_SIZE * 2U)
#define BL_GET_QUOTIENT(x, y)                           ((x) / (y))
#define BL_GET_REMAINDER(x, y)                          ((x) % (y))
typedef void (*jump_func)(void);

static eBL_PROCESS_STEP s_bl_process_step;

//static unUINT32_B s_bl_sw_version;
static stSOFT_TIMER s_bl_common_timer;
static stSOFT_TIMER s_bl_app_dw_timer;
static stSOFT_TIMER s_bl_go_app_timer;
static stSOFT_TIMER s_bl_go_reset_timer;
static uint08 s_flash_buf[WR_UNIT_BYTE_NUM];
static uint08 s_flash_wrbuf[DCM_RX_BUF_SIZE];
static stBL_INFO s_bl_info;
eBOOT_APP_CHG_STS g_boot_app_chg_sts;

static typ_bool bl_flash_write(uint32 addr, uint08* data, uint32 len);
static typ_bool bl_flash_read(uint32 addr, uint08* data, uint32 len);
static typ_bool bl_flash_erase(uint32 addr, uint32 size);
//static typ_bool chk_programming_preconditions(void);
//static typ_bool chk_app_valid_crc(void);
//static void bl_ecu_reset(void);
static void jump2app(void);
//static void jump2boot(void);
static void preparation_before_jump(void);

static typ_bool bl_flash_write(uint32 addr, uint08* data, uint32 len)
{
    if (len < DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH)
    {
        len = DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH;
    }
//    WDOG_FEED();
    
    return (s32k144_flash_write(addr, data, len));
}

static typ_bool bl_flash_read(uint32 addr, uint08* data, uint32 len)
{
    return (s32k144_flash_read(addr, data, len));
}

typ_bool bl_flash_erase(uint32 addr, uint32 size)
{
//    return DEF_TRUE;
//    WDOG_FEED();
    return (s32k144_flash_erase_sector(addr, size));
}

//static typ_bool chk_programming_preconditions(void)
//{
//    return DEF_TRUE;
//}

//static typ_bool chk_app_valid_crc(void)
//{
//    return DEF_TRUE;
//}

//static void bl_ecu_reset(void)
//{
//    
//}

static void jump2app(void)
{
    jump_func jumpapp;
    uint32 jumpstack;

//    jumpapp = (jump_func)(APP_A_START_ADDR + 4U);
//    jumpstack = APP_A_START_ADDR;
//    S32_SCB->VTOR = (uint32)APP_A_START_ADDR;
//	__asm volatile ("MSR msp, %0\n" : : "r" (jumpstack) : "sp");
//	__asm volatile ("MSR psp, %0\n" : : "r" (jumpstack) : "sp");
//    jumpapp();
}

//static void jump2boot(void)
//{
//    #if 1
//    jump_func jumpboot;
//    uint32 jumpstack;
//
//    jumpboot = (jump_func)(BOOT_START_ADDR + 4U);
//    jumpstack = BOOT_START_ADDR;
//    S32_SCB->VTOR = (uint32)BOOT_START_ADDR;
//	__asm volatile ("MSR msp, %0\n" : : "r" (jumpstack) : "sp");
//	__asm volatile ("MSR psp, %0\n" : : "r" (jumpstack) : "sp");
//    jumpboot();
//    #else
//    SystemSoftwareReset();
//    #endif
//}

static void preparation_before_jump(void)
{
    uint32 i = 0;

//    INT_SYS_DisableIRQGlobal();
//    /* close systick */
//    S32_SysTick->CSR = 0;
//    S32_SysTick->RVR = 0;
//    S32_SysTick->CVR = 0;
//    
//    /* close all interrupt */
//    for (i = 0; i < 8U; i++)
//    {
//        S32_NVIC->ICER[i] = 0xFFFFFFFF;
//        S32_NVIC->ICPR[i] = 0xFFFFFFFF;
//    }
//    INT_SYS_EnableIRQGlobal();
}

static void bl_ecu_reset(void)
{
//    SystemSoftwareReset();
      Cy_SysReset_SoftResetCM4();
      NVIC_SystemReset();

}
void bootloader_init(void)
{
    s_bl_process_step = BL_STEP_RESET;
    common_memset((uint08*)s_flash_buf, WR_UNIT_BYTE_NUM, 0U);
    common_memset((uint08*)s_flash_wrbuf, DCM_RX_BUF_SIZE, 0U);
    common_memset((uint08*)&s_bl_info, 0U, sizeof(stBL_INFO));
}

void bootloader_main_process(void)
{
    unUINT32_B tempb = {0U};
    uint32 i = 0;
    typ_bool ret = DEF_FALSE;
    eBOOT_APP_CHG_STS chg_sts = BOOT_APP_CHG_INIT;

    eBL_PROCESS_STEP bl_ps = s_bl_process_step;
    switch (bl_ps)
    {
        case BL_STEP_RESET:
//            bl_ps = BL_STEP_SW_EXTENDED_SESSION;
            bl_ps = BL_STEP_CHK_PRE_PROG_CONDITION_PENDING;
            soft_timer_set(&s_bl_common_timer, BOOT_COMM_TIME);
            tempb.bytes = 0;
            eeprom_drv_read(APP_BOOT_CHG_FLAG_ADDR, (uint08*)&tempb, 4U); /* read change status */
            g_boot_app_chg_sts = (eBL_PROCESS_STEP)tempb.bytes;
            break;
        case BL_STEP_CHK_UPDATE_FLG:
            tempb.bytes = APP_UPDATE_FLAG;
            eeprom_drv_write(APP_UPDATE_FLAG_ADDR, (uint08*)&tempb, 4U); /* write update flag */
            g_boot_app_chg_sts = APP_2_BOOT_1002;
            write_boot_app_chg_sts_2_e2(g_boot_app_chg_sts);
            uint32_t jump_flag = TO_BOOT_FLAG;
            eeprom_drv_write(APP_BOOT_JUMP_FLAG_ADDR, (uint08*)&jump_flag, 4U); /* clear flag */
            Flash_Task_Wait(); /* the flags must be written before the ECU resets */
            bl_ps = BL_STEP_ECU_RESET;
            soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            break;
        case BL_STEP_GOTO_BOOT:
            bl_ps = BL_STEP_ECU_RESET;
            soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            break;
        case BL_STEP_CHK_APP_VALID:
            tempb.bytes = 0;
            eeprom_drv_read(APP_VALID_ADDR, (uint08*)&tempb, 4U); /* read app valid flag */
            if (0 != s_bl_info.update_flg) /* upgrade request */
            {
                bl_ps = BL_STEP_REQUEST_SEED;
                dcm_unconditional_session_ctrl(SESSION_TYPE_PROGRAMMING);
            }
//            else if (APP_VALID_VAL != tempb.bytes) /* app invalid */
//            {
//                bl_ps = BL_STEP_SW_PROGRAMMING_SESSION; 
//            }
            else
            {
                bl_ps = BL_STEP_GOTO_APP; /* no upgrade request and the app is valid, goto app */
            }
            break;
        case BL_STEP_SW_EXTENDED_SESSION:
            break;
        case BL_STEP_CHK_PRE_PROG_CONDITION:
            bl_ps = BL_STEP_STOP_DTC;
            break;
        case BL_STEP_STOP_DTC:
            bl_ps = BL_STEP_BLOCK_USELESS_COM;
            break;
        case BL_STEP_BLOCK_USELESS_COM:
            bl_ps = BL_STEP_SW_PROGRAMMING_SESSION;
            break;
        case BL_STEP_READ_VERSION:
            break;
        case BL_STEP_SW_PROGRAMMING_SESSION:
            break;
        case BL_STEP_REQUEST_SEED:
            bl_ps++;
            break;
        case BL_STEP_RESPONSE_KEY:
            bl_ps++;
            break;
        case BL_STEP_WRITING_FINGERPRINT:
            bl_ps = BL_STEP_ERASE_MEMORY_PENDING;
            break;
        case BL_STEP_DOWNLOAD_FLASHDRIVER:
            break;
        case BL_STEP_CHK_FLASHDRIVER_DEPENDENCIES:
            break;
        case BL_STEP_ERASE_MEMORY:
            if (DEF_TRUE == bl_flash_erase(APP_M0_A_START_ADDR, APP_M0_A_END_ADDR + 1U - APP_M0_A_START_ADDR))
            {
                bl_ps = BL_STEP_DOWNLOAD_APP_PENDING;
                soft_timer_set(&s_bl_app_dw_timer, BL_APP_DW_TIME);
            }
            else
            {
                bl_ps = BL_STEP_ERASE_MEMORY_REPEAT_PENDING;
            }
            break;
        case BL_STEP_DOWNLOAD_APP:
            bl_ps = BL_STEP_CHK_APP_DEPENDENCIES_PENDING;
            break;
        case BL_STEP_CHK_APP_DEPENDENCIES:
            bl_ps = BL_STEP_GOTO_APP;
            soft_timer_set(&s_bl_go_app_timer, BL_GO_APP_TIME);
            break;
        case BL_STEP_GOTO_APP:
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_go_app_timer))
            {               
                preparation_before_jump();
                jump2app();
            }
            else {}
            break;
        case BL_STEP_ECU_RESET:
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_go_reset_timer))
            {
                bl_ecu_reset();
            }
            else {}
            break;
        case BL_STEP_CHK_PRE_PROG_CONDITION_PENDING:
            break;
        case BL_STEP_CHK_PRE_PROG_CONDITION_REPEAT_PENDING:
            break;
        case BL_STEP_DOWNLOAD_FLASHDRIVER_REPEAT_PENDING:
            break;
        case BL_STEP_ERASE_MEMORY_PENDING:
            break;
        case BL_STEP_ERASE_MEMORY_REPEAT_PENDING:
            bl_ps = BL_STEP_ERASE_MEMORY;
            break;
        case BL_STEP_DOWNLOAD_APP_PENDING:
            soft_timer_reset(&s_bl_common_timer);
            break;
        case BL_STEP_CHK_APP_DEPENDENCIES_PENDING:
            break;
        case BL_STEP_WAIT_1002_RES_PENDING:
            break;
        default:
            break;
    }
    s_bl_process_step = bl_ps;

    if (s_bl_process_step < BL_STEP_ECU_RESET)
    {
        soft_timer_reset(&s_bl_common_timer);
    }
    else
    {
        if (BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step)
        {
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_app_dw_timer))
            {
                s_bl_process_step = BL_STEP_ECU_RESET;
                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            }
            else {}
        }
        else
        {
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_common_timer))
            {
//                s_bl_process_step = BL_STEP_RESET;
            }
            else {}
        }

//        if ((dcm_session_get() != SESSION_TYPE_EXTENDED) && (dcm_session_get() != SESSION_TYPE_PROGRAMMING))
//        {
//            s_bl_process_step = BL_STEP_RESET;
//        }
//        else {}
    }
}

void bl_s3_timeout_cbk(void)
{
    if (s_bl_info.app_valid_flg)
    {
        g_boot_app_chg_sts = BOOT_2_APP_S3_TIMEOUT;
        write_boot_app_chg_sts_2_e2(g_boot_app_chg_sts);
        s_bl_process_step = BL_STEP_GOTO_APP;
        soft_timer_set(&s_bl_go_app_timer, BL_GO_APP_TIME);
    }

}
void bl_service_10_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    unUINT32_B tempb = {0U};

    (void)cbk_param2;
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        if (DCM_SID10_SUB_03 == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
        {
            if (BL_STEP_SW_EXTENDED_SESSION == s_bl_process_step)
            {
                s_bl_process_step = BL_STEP_CHK_PRE_PROG_CONDITION_PENDING;
            }
        }
        else if (DCM_SID10_SUB_02 == (msg->req_data[1] & (~SUPPRESS_POS_RESP_MSG_INDICATION_BIT)))
        {
            if (BL_STEP_SW_PROGRAMMING_SESSION == s_bl_process_step)
            {
                s_bl_process_step = BL_STEP_CHK_UPDATE_FLG;
                msg->nrc = NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING;
            }
            else
            {
                #if  0
                tempb.bytes = APP_SESSION_SW_ONLY;
                eeprom_drv_write(APP_UPDATE_FLAG_ADDR, (uint08*)&tempb, 4U); /* write update flag */
                s_bl_process_step = BL_STEP_ECU_RESET; /* If the steps are incorrect, the upgrade flag will not be written. */
                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
                #else
                s_bl_process_step = BL_STEP_CHK_UPDATE_FLG;//BL_STEP_GOTO_BOOT;
                msg->nrc = NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING;
                #endif
            }
        }
        else { /* nothing */ }
    }
    else { /* nothing */ }
}

void bl_service_28_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    
    (void)cbk_param2;
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        
    }
    else { /* nothing */ }
}

void bl_service_31_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    
    (void)cbk_param2;
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        
    }
    else { /* nothing */ }
}

void bl_service_34_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    uint32 temp = 0;

    if (   (NRC_POSITIVE_RESPONSE == msg->nrc) 
        && ((BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step) || (BL_STEP_DOWNLOAD_FLASHDRIVER == s_bl_process_step))
        )
    {
        common_memcpy((uint08*)&s_bl_info.sid34_dp, (uint08*)cbk_param2, sizeof(stSID34_DP_INFO));
        s_bl_info.block_cnt = 0;
        temp = (s_bl_info.sid34_dp.wr_data_size % DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH);
        if (temp)
        {
            s_bl_info.block_total = s_bl_info.sid34_dp.wr_data_size / DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH + 1;
        }
        else
        {
            s_bl_info.block_total = s_bl_info.sid34_dp.wr_data_size / DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH;
            temp = DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH;
        }
        s_bl_info.block_len = DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH;
        s_bl_info.tail_block_len = temp;
        s_bl_info.recv_addr = s_bl_info.sid34_dp.mem_start_addr;
    }
    else { /* nothing */ }
}

void bl_service_36_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;

    if (   (NRC_POSITIVE_RESPONSE == msg->nrc) 
        && ((BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step) || (BL_STEP_DOWNLOAD_FLASHDRIVER == s_bl_process_step))
        )
//    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        common_memcpy((uint08*)&s_bl_info.sid36_dp, (uint08*)cbk_param2, sizeof(stSID36_DP_INFO));
        if (s_bl_info.sid36_dp.block_seq_cnt < 0xFF)
        {
            s_bl_info.sid36_dp.block_seq_cnt++;
        }
        else
        {
            s_bl_info.sid36_dp.block_seq_cnt = 0;
        }
        common_memcpy((uint08*)s_flash_wrbuf, (uint08*)(msg->req_data + 2U), s_bl_info.block_len);
        s_bl_info.block_cnt++;
        if (s_bl_info.block_cnt >= s_bl_info.block_total)
        {
            bl_flash_write(s_bl_info.recv_addr, (uint08*)s_flash_wrbuf, s_bl_info.tail_block_len);
            s_bl_info.recv_addr += s_bl_info.tail_block_len;
            s_bl_info.sid36_dp.recv_over = 1;
        }
        else
        {
            bl_flash_write(s_bl_info.recv_addr, (uint08*)s_flash_wrbuf, s_bl_info.block_len);
            s_bl_info.recv_addr += s_bl_info.block_len;
        }       
        
        s_bl_info.pre_block_seq_cnt = s_bl_info.sid36_dp.block_seq_cnt;
        common_memcpy((uint08*)cbk_param2, (uint08*)&s_bl_info.sid36_dp, sizeof(stSID36_DP_INFO));
    }
    else { /* nothing */ }
}

void bl_service_37_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    
    (void)cbk_param2;

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        if (BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step)
        {
            s_bl_process_step = BL_STEP_DOWNLOAD_APP;
        }
        else if (BL_STEP_DOWNLOAD_FLASHDRIVER == s_bl_process_step)
        {
            s_bl_process_step = BL_STEP_CHK_FLASHDRIVER_DEPENDENCIES;
        }
        else { /* nothing */ }
    }
    else { /* nothing */ }
}

uint08 routine_erase_memory(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    if (BL_STEP_ERASE_MEMORY_PENDING == s_bl_process_step)
    {
        s_bl_process_step = BL_STEP_ERASE_MEMORY;
    }
    else { /* nothing */ }
    return nrc;
}

uint08 routine_check_programming_dependencies(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    if (BL_STEP_CHK_APP_DEPENDENCIES_PENDING == s_bl_process_step)
    {
        s_bl_process_step = BL_STEP_CHK_APP_DEPENDENCIES;
    }
    else { /* nothing */ }
    return nrc;
}

uint08 routine_check_programming_pre_conditions(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    if (BL_STEP_CHK_PRE_PROG_CONDITION_PENDING == s_bl_process_step)
    {
//        s_bl_process_step = BL_STEP_CHK_PRE_PROG_CONDITION;
        s_bl_process_step = BL_STEP_SW_PROGRAMMING_SESSION;
    }
    else { /* nothing */ }
    return nrc;
}

uint08 routine_check_programming_integrity(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    return nrc;
}

void write_boot_app_chg_sts_2_e2(eBOOT_APP_CHG_STS sts)
{
    unUINT32_B tempb = {0U};
    
    tempb.bytes = sts;
    eeprom_drv_write(APP_BOOT_CHG_FLAG_ADDR, (uint08*)&tempb, 4U); /* write change status */
}

uint8_t t_pcan_data_send[8] = {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80};
void pcan_test_can_send(void)
{
    can_transmit(0, 0x18FEEC19, 8, t_pcan_data_send);
}

void bl_printf(void)
{
    DEF_PRINTF_TIME(RTT_CTRL_TEXT_GREEN"s_bl_process_step: %d.\r\n", s_bl_process_step);
}

