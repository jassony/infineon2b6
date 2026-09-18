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
#include "m0_boot_pdma_crc.h"
//#include "Cpu.h"
//#include "system_S32K144.h"
#include "crc32.h"
//#include "peripheral_include.h"
#include "can_dcm.h"
#include "uds_user.h"
#define BL_WDG_FEED
#define WR_UNIT_BYTE_NUM                                DCM_SID34_MAX_NUMBER_OF_BLOCK_LENGTH
#define RD_UNIT_BYTE_NUM                                1024U
#define BL_WR_BUF_SIZE                                  (DCM_RX_BUF_SIZE * 2U)
#define BL_GET_QUOTIENT(x, y)                           ((x) / (y))
#define BL_GET_REMAINDER(x, y)                          ((x) % (y))
#define PFLASH_PAGE_NUM                                 CY_WORK_SES_SIZE_IN_BYTE
typedef void (*jump_func)(void);

static eBL_PROCESS_STEP s_bl_process_step;
static stSOFT_TIMER s_bl_common_timer;
static stSOFT_TIMER s_bl_go_app_timer;
static stSOFT_TIMER s_bl_app_dw_timer;
static stSOFT_TIMER s_bl_go_reset_timer;
static uint08 s_flash_buf[RD_UNIT_BYTE_NUM];
static uint08 s_flash_wrbuf[DCM_RX_BUF_SIZE];
static uint08 s_pflash_tmpbuf[PFLASH_PAGE_NUM];
static stBL_INFO s_bl_info;
static uint08 s_flash_driver_func[256U];
static uint32 s_bl_init_crc;
static uint32 s_bl_recv_crc;
static stSID31_DP_INFO s_bl_sid31_dp;
static uint08 s_flashdrive_flag ;
eBOOT_APP_CHG_STS g_boot_app_chg_sts;

static typ_bool bl_flash_write(uint32 addr, uint08* data, uint32 len);
static typ_bool bl_flash_read(uint32 addr, uint08* data, uint32 len);
static typ_bool bl_flash_erase(uint32 addr, uint32 size);
static typ_bool pflash_page_write(uint32 addr, uint08* data, uint32 len);
static typ_bool chk_programming_preconditions(void);
static void bl_ecu_reset(void);
static void jump2app(void);
static void preparation_before_jump(void);

extern void BACK_TO_START(void);
extern volatile uint32 s_system_ticks;

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

static typ_bool bl_flash_erase(uint32 addr, uint32 size)
{
//    WDOG_FEED();
    return (s32k144_flash_erase_sector(addr, size));
}

static typ_bool pflash_page_write(uint32 addr, uint08* data, uint32 len)
{
    uint32 ppage_addr = 0;
    uint16 addr_remainder = (addr % PFLASH_PAGE_NUM);
    typ_bool ret = DEF_FALSE;
    
    if ((addr_remainder + len) > PFLASH_PAGE_NUM)
    {
        return DEF_FALSE;
    }

    ppage_addr = addr - addr_remainder;
    (void)s32k144_flash_read(ppage_addr, (uint08*)s_pflash_tmpbuf, PFLASH_PAGE_NUM);
    (void)s32k144_flash_erase_sector(ppage_addr, PFLASH_PAGE_NUM);
//    WDOG_FEED();
    common_memcpy((uint08*)&s_pflash_tmpbuf[addr_remainder], (uint08*)data, len);
    ret = s32k144_flash_write(ppage_addr, (uint08*)s_pflash_tmpbuf, PFLASH_PAGE_NUM);
//    WDOG_FEED();

    return ret;
}

static typ_bool chk_programming_preconditions(void)
{
    return DEF_TRUE;
}

typ_bool chk_app_valid_crc(void)
{
    if (s_bl_recv_crc == s_bl_init_crc)
    {
        return DEF_TRUE;
    }
    else
    {
        return DEF_FALSE;
    }
}

static void bl_ecu_reset(void)
{
//    SystemSoftwareReset();
    NVIC_SystemReset();

}

/***************************************************************************************
 * Function		: Bootup_Application
 * Description	: Bootup_Application
 * Input	    : appEntry,appStack
 * Output		: None
 ***************************************************************************************/
static void Bootup_Application(uint32_t appEntry, uint32_t appStack)
{
//	static void (*jump_to_application)(void);
//	static uint32_t stack_pointer;
//
//	jump_to_application = (void (*)(void))appEntry;
//	stack_pointer = appStack;
//	S32_SCB->VTOR = (uint32_t)APP_A_START_ADDR;
//	__asm volatile ("MSR msp, %0\n" : : "r" (stack_pointer) : "sp");
//	__asm volatile ("MSR psp, %0\n" : : "r" (stack_pointer) : "sp");
//	jump_to_application();
}

static void jump2app(void)
{
    uint32_t appEntry, appStack;
    uint32_t jump_flag = TO_APP_FLAG;

    eeprom_drv_write(APP_BOOT_JUMP_FLAG_ADDR, (uint08*)&jump_flag, 4U); /* clear flag */
    NVIC_SystemReset();
//    appStack = *(uint32_t *)(APP_A_START_ADDR);
//	appEntry = *(uint32_t *)(APP_A_START_ADDR + 4);
//	Bootup_Application(appEntry, appStack);
}

static void preparation_before_jump(void)
{
    uint32 i = 0;
    
//    can_drv_deinit(CAN_CHN_0);
//    can_drv_deinit(CAN_CHN_2);
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

/* app flash read back(72092 bytes) and CRC32 calculation take about 41ms */
static typ_bool bl_app_readback_crc_chk(uint32 app_start_addr, uint32 app_len, uint32 read_crc)
{
    uint32 cal_crc = 0;
    uint32 rd_addr = 0;
    uint32 block_total = 0;
    uint32 block_len = 0;
    uint32 tail_block_len = 0;
    uint32 block_cnt = 0;
    typ_bool ret = DEF_FALSE;

    tail_block_len = (app_len % RD_UNIT_BYTE_NUM);
    if (tail_block_len)
    {
        block_total = app_len / RD_UNIT_BYTE_NUM + 1;
    }
    else
    {
        block_total = app_len / RD_UNIT_BYTE_NUM;
        tail_block_len = RD_UNIT_BYTE_NUM;
    }
    block_len = RD_UNIT_BYTE_NUM;
    rd_addr = app_start_addr;
    while (1)
    {
        block_cnt++;
        if (block_cnt >= block_total)
        {
            bl_flash_read(rd_addr, (uint08*)s_flash_buf, tail_block_len);
            cal_crc = Pdma_Crc32_Cal(s_flash_buf, tail_block_len);
            rd_addr += tail_block_len;
            break;
        }
        else
        {
            bl_flash_read(rd_addr, (uint08*)s_flash_buf, block_len);
            cal_crc = Pdma_Crc32_Cal( s_flash_buf, block_len);
            rd_addr += block_len;
        }
        
//        WDOG_FEED();
    }

    if (read_crc == cal_crc)
    {
        return DEF_TRUE;
    }
    else
    {
        return DEF_FALSE;
    }
}

//static void bl_app_b2a_copy(void)
//{
//    if (APP_A_TOTAL_SIZE != APP_B_TOTAL_SIZE)
//    {
//        return;
//    }
//    
//    (void)bl_flash_erase(APP_A_START_ADDR, APP_A_TOTAL_SIZE);
//    bl_flash_write(APP_A_START_ADDR, (uint08*)APP_B_START_ADDR, APP_B_TOTAL_SIZE);
//}
//
//static void bl_app_a2b_copy(void)
//{
//    if (APP_A_TOTAL_SIZE != APP_B_TOTAL_SIZE)
//    {
//        return;
//    }
//    
//    (void)bl_flash_erase(APP_B_START_ADDR, APP_A_TOTAL_SIZE);
//    bl_flash_write(APP_B_START_ADDR, (uint08*)APP_A_START_ADDR, APP_B_TOTAL_SIZE);
//}

static void bl_app_valid_process(void)
{
    #if 0
    unUINT32_B tempb = {0U};
    uint32 code_len = 0;
    uint32 read_crc = 0;
    
    tempb.bytes = 0;
    eeprom_drv_read(APP_A_LEN_ADDR, (uint08*)&tempb, 4U);
    code_len = tempb.bytes;

    tempb.bytes = 0;
    eeprom_drv_read(APP_A_CRC_ADDR, (uint08*)&tempb, 4U);
    read_crc = tempb.bytes;

    if (DEF_TRUE == bl_app_readback_crc_chk(APP_A_START_ADDR, code_len, read_crc))
    {
        s_bl_info.app_valid_flg = 1;
    }
    else
    {
        tempb.bytes = 0;
        eeprom_drv_read(APP_B_LEN_ADDR, (uint08*)&tempb, 4U);
        code_len = tempb.bytes;

        tempb.bytes = 0;
        eeprom_drv_read(APP_B_CRC_ADDR, (uint08*)&tempb, 4U);
        read_crc = tempb.bytes;
        if (DEF_FALSE == bl_app_readback_crc_chk(APP_B_START_ADDR, code_len, read_crc))
        {
            tempb.bytes = code_len;
            eeprom_drv_write(APP_A_LEN_ADDR, (uint08*)&tempb, 4U);
            tempb.bytes = read_crc;
            eeprom_drv_write(APP_A_CRC_ADDR, (uint08*)&tempb, 4U);
            bl_app_b2a_copy();
            s_bl_info.app_valid_flg = 1;
        }
        else
        {
            s_bl_info.app_valid_flg = 0;
        }
    }
    #endif
}

static void bl_31_nrc78_res(PDU_ID hrh, uint32 nrc,uint32_t RID)
{
    uint08 buf[DCM_RX_NRC78_BUF_SIZE] = {0};

    if (NRC_POSITIVE_RESPONSE == nrc)
    {
        buf[0] = DCM_SID_31;
        buf[1] = DCM_SID31_SUB_01;
        buf[2] = (uint08)(RID >> 8U);
        buf[3] = (uint08)(RID >> 0U);
      if(RID == RID_ERASE_MEMORY)
      {
        buf[4] = 1;
        dcm_nrc78_posres_set(hrh, buf, 5U);
      }
      else
        dcm_nrc78_posres_set(hrh, buf, 4u);

    }
    else
    {
        dcm_nrc78_negres_set(hrh, nrc);
    }
}

void bootloader_init(void)
{
    s_bl_process_step = BL_STEP_RESET;
    common_memset((uint08*)s_flash_buf, RD_UNIT_BYTE_NUM, 0U);
    common_memset((uint08*)s_flash_wrbuf, DCM_RX_BUF_SIZE, 0U);
    common_memset((uint08*)s_pflash_tmpbuf, PFLASH_PAGE_NUM, 0U);
    common_memset((uint08*)&s_bl_info, 0U, sizeof(stBL_INFO));
    common_memset((uint08*)&s_bl_sid31_dp, 0U, sizeof(stSID31_DP_INFO));
    s_bl_init_crc = 0;
}

void bootloader_main_process(void)
{
    unUINT32_B tempb = {0U};
    uint32 i = 0;
    typ_bool ret = DEF_FALSE;

    eBL_PROCESS_STEP bl_ps = s_bl_process_step;
    switch (bl_ps)
    {
        case BL_STEP_RESET:
            bl_ps = BL_STEP_CHK_UPDATE_FLG;
//            CAN_DEBUG_PRINT("[BL-MAIN]boot step[RESET].\r\n");
            break;
        case BL_STEP_CHK_UPDATE_FLG:
            tempb.bytes = 0;
            s_flashdrive_flag = 0;
            eeprom_drv_read(APP_UPDATE_FLAG_ADDR, (uint08*)&tempb, 4U); /* read update flag */
            if (APP_UPDATE_FLAG == tempb.B.byte0)
            {
                s_bl_info.update_flg = 1;
                tempb.bytes = 0xFFFFFFFF;
                eeprom_drv_write(APP_UPDATE_FLAG_ADDR, (uint08*)&tempb, 4U); /* clear flag */


//                CAN_DEBUG_PRINT("[BL-MAIN]update flag = 1.\r\n");
            }
            else
            {
                s_bl_info.update_flg = 0;
//                CAN_DEBUG_PRINT("[BL-MAIN]update flag = 0.\r\n");
            }
            bl_ps = BL_STEP_CHK_APP_VALID;
            break;
        case BL_STEP_CHK_APP_VALID:
            tempb.bytes = 0;
            bl_flash_read(APP_M0_A_START_ADDR, (uint08*)&tempb, 4U); /* read app A valid flag */
            if (0xffffffff != tempb.bytes) /* app invalid */
            {
                s_bl_info.app_valid_flg = 1;
            }
            else
            {
                #ifdef APP_BACKUP_EN
                tempb.bytes = 0;
                bl_flash_read(APP_B_VALID_ADDR, (uint08*)&tempb, 4U); /* read app B valid flag */
                if (APP_VALID_VAL == tempb.bytes) /* app invalid */
                {
                    bl_app_b2a_copy();
                    tempb.bytes = APP_VALID_VAL;
                    pflash_page_write(APP_A_VALID_ADDR, (uint08*)&tempb, 4U);
                    s_bl_info.app_valid_flg = 1;
                }
                else
                #endif
                {
                    s_bl_info.app_valid_flg = 0;
                    g_boot_app_chg_sts = BOOT_APP_CHG_INIT;
                    write_boot_app_chg_sts_2_e2(g_boot_app_chg_sts);
                }
            }
            
            if (0 != s_bl_info.update_flg) /* upgrade request */
            {
                bl_ps = BL_STEP_REQUEST_SEED;
                dcm_unconditional_session_ctrl(SESSION_TYPE_PROGRAMMING);
                tempb.bytes = 0;
                eeprom_drv_read(APP_BOOT_CHG_FLAG_ADDR, (uint08*)&tempb, 4U); /* read change status */
                g_boot_app_chg_sts = (eBL_PROCESS_STEP)tempb.bytes;
                if (BOOT_APP_CHG_INIT != g_boot_app_chg_sts)
                {
                    if (APP_2_BOOT_1002 == g_boot_app_chg_sts)
                    {
                        dcm_1002_nrc78_form_app_set(CANIF_HRH0_PHY);
                    }
                    g_boot_app_chg_sts = BOOT_APP_CHG_INIT;
                    write_boot_app_chg_sts_2_e2(g_boot_app_chg_sts);
                    
                }
                else
                {    
                }
            }
            else if (0 == s_bl_info.app_valid_flg) /* app invalid */
            {
                bl_ps = BL_STEP_SW_PROGRAMMING_SESSION_PENDING; 
//                CAN_DEBUG_PRINT("[BL-MAIN]app invalid.\r\n");
            }
            else
            {
                dcm_unconditional_session_ctrl(SESSION_TYPE_PROGRAMMING);
                tempb.bytes = 0xFFFFFFFF;
                eeprom_drv_write(APP_UPDATE_FLAG_ADDR, (uint08*)&tempb, 4U); /* clear flag */
                bl_ps = BL_STEP_GOTO_APP; /* no upgrade request and the app is valid, goto app */
                soft_timer_set(&s_bl_go_app_timer, BL_GO_APP_TIME);
//                CAN_DEBUG_PRINT("[BL-MAIN]no upgrade request and the app is valid, goto app.\r\n");
            }
            break;
        case BL_STEP_SW_EXTENDED_SESSION:
            break;
        case BL_STEP_CHK_PRE_PROG_CONDITION:
            break;
        case BL_STEP_STOP_DTC:
            break;
        case BL_STEP_BLOCK_USELESS_COM:
            break;
        case BL_STEP_READ_VERSION:
            break;
        case BL_STEP_SW_PROGRAMMING_SESSION:
            bl_ps++;
            break;
        case BL_STEP_REQUEST_SEED:
            bl_ps++;
            break;
        case BL_STEP_RESPONSE_KEY:
            bl_ps++;
            break;
        case BL_STEP_WRITING_FINGERPRINT:
            bl_ps = BL_STEP_DOWNLOAD_FLASHDRIVER_PENDING;
            soft_timer_set(&s_bl_app_dw_timer, BL_APP_DW_TIME);
            break;
        case BL_STEP_DOWNLOAD_FLASHDRIVER:
            break;
        case BL_STEP_CHK_FLASHDRIVER_INTEGRITY:
            bl_ps = BL_STEP_ERASE_MEMORY_PENDING;
            soft_timer_set(&s_bl_common_timer, BOOT_COMM_TIME);
            break;
        case BL_STEP_ERASE_MEMORY:
            tempb.bytes = 0;
            if (1 == s_bl_info.mem_erase_nrc78_flg)
            {
                s_bl_info.mem_erase_nrc78_flg = 0;
                tempb.bytes = 0;
//                bl_31_nrc78_res(s_bl_info.mem_erase_hrh, NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING);
//            }
//            else if (2 == s_bl_info.mem_erase_nrc78_flg)
//            {
              s_bl_info.mem_erase_nrc78_flg = 0;
              if (DEF_TRUE == bl_flash_erase(APP_PREPROGRAM_START_ADDR, APP_PREPROGRAM_TOTAL_SIZE))
              {
                  bl_ps = BL_STEP_DOWNLOAD_APP_PENDING;
                  soft_timer_set(&s_bl_app_dw_timer, BL_APP_DW_TIME);
                  bl_31_nrc78_res(s_bl_info.mem_erase_hrh, NRC_POSITIVE_RESPONSE,RID_ERASE_MEMORY);
                  if(did_0201_val[0] < 0xff)
                    did_0201_val[0] += 1;
                  eeprom_drv_write(DID_0201_ADDR, (uint08*)&did_0201_val, DID_0201_DATA_LEN); /* clear flag */
              }
              else
              {
                  bl_ps = BL_STEP_ECU_RESET;
                  bl_31_nrc78_res(s_bl_info.mem_erase_hrh, NRC_GENERAL_PROGRAMMING_FALIURE,RID_ERASE_MEMORY);
                  soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
              }
            }
//            bl_flash_read(APP_A_VALID_ADDR, (uint08*)&tempb, 4U); /* read app A valid flag */
//            if (APP_VALID_VAL == tempb.bytes) /* app valid */
//            {
//                #ifdef APP_BACKUP_EN
//                bl_app_a2b_copy();
//                tempb.bytes = APP_VALID_VAL;
//                pflash_page_write(APP_B_VALID_ADDR, (uint08*)&tempb, 4U);
//                #endif
//            }
//            else {}
                
//            tempb.bytes = 0xFFFFFFFF;
//            pflash_page_write(APP_A_VALID_ADDR, (uint08*)&tempb, 4U); /* app invalid */
            
            break;
        case BL_STEP_DOWNLOAD_APP:
            bl_ps = BL_STEP_DOWNLOAD_INTEGRITY_PENDING;
            break;
        case BL_STEP_DOWNLOAD_INTEGRITY:
            if (1 == s_bl_info.mem_erase_nrc78_flg)
            {
//                s_bl_info.mem_erase_nrc78_flg = 2;
//                bl_31_nrc78_res(s_bl_info.mem_erase_hrh, NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING);

//            }
//            else
//            {
              s_bl_info.mem_erase_nrc78_flg = 0;
              if (DEF_TRUE == chk_app_valid_crc())
              {
                  bl_flash_erase(s_bl_info.recv_addr, s_bl_info.sid34_dp.wr_data_size);
                  bl_flash_write(s_bl_info.recv_addr, (uint08*)APP_PREPROGRAM_START_ADDR, s_bl_info.sid34_dp.wr_data_size);
                  if(*(uint16_t *)did_0200_val < 0xffff)
                    *(uint16_t *)did_0200_val += 1;
                  did_0201_val[0] = 0;
                  uint8_t data[DID_0200_DATA_LEN + DID_0201_DATA_LEN];
                  common_memcpy(data, did_0200_val, DID_0200_DATA_LEN);
                  common_memcpy(data + DID_0200_DATA_LEN, did_0201_val, DID_0201_DATA_LEN);
                  eeprom_drv_write(DID_0200_ADDR, (uint08*)&data, DID_0200_DATA_LEN + DID_0201_DATA_LEN); /* clear flag */
                  bl_ps = BL_STEP_CHK_APP_DEPENDENCIES_PENDING;
                  bl_31_nrc78_res(s_bl_info.mem_erase_hrh, NRC_POSITIVE_RESPONSE,RID_CHECK_PROGRAMMING_INTEGRITY);
              }
              else
              {
                  bl_ps = BL_STEP_ECU_RESET;
                  soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
                  bl_31_nrc78_res(s_bl_info.mem_erase_hrh, NRC_GENERAL_PROGRAMMING_FALIURE,RID_CHECK_PROGRAMMING_INTEGRITY);

              }
            }
            break;
        case BL_STEP_CHK_APP_DEPENDENCIES://依赖性检查  这个需要放到rid回调函数中去做，因为上位机那边收到正响应会直接发11 01复位，导致这里会执行不到就复位了
//            tempb.bytes = 0;
//            if(APP_USER_M0_SPC_ADDR == s_bl_info.recv_addr)
//              bl_flash_read(APP_USER_M0_SPC_ADDR, (uint08*)&tempb, APP_USER_SPC_SIZE);
//            else
//              bl_flash_read(APP_USER_M4_SPC_ADDR, (uint08*)&tempb, APP_USER_SPC_SIZE);
//            if (   (APP_USER_SPC_VAL == tempb.B.byte0)
//                && (APP_USER_SPC_VAL == tempb.B.byte1)
//                && (APP_USER_SPC_VAL == tempb.B.byte2)
//                && (APP_USER_SPC_VAL == tempb.B.byte3)
//                )
//            {
////                tempb.bytes = APP_VALID_VAL;
////                pflash_page_write(APP_A_VALID_ADDR, (uint08*)&tempb, 4U); /* write app valid flag */
//				#ifdef APP_BACKUP_EN 
//                tempb.bytes = 0;
//                bl_flash_read(APP_B_VALID_ADDR, (uint08*)&tempb, 4U); /* read app B valid flag */
//                if (APP_VALID_VAL != tempb.bytes) /* app invalid */
//                {
//                    bl_app_a2b_copy();
//                    tempb.bytes = APP_VALID_VAL;
//                    pflash_page_write(APP_B_VALID_ADDR, (uint08*)&tempb, 4U);
//                }
//                else {}
//                #endif
////                bl_ps = BL_STEP_GOTO_APP;
//                bl_ps = BL_STEP_WAIT_FOR_REST;
//                soft_timer_set(&s_bl_go_app_timer, BL_RESET_WAIT_TIME);
//            }
//            else
//            {
//                bl_ps = BL_STEP_ECU_RESET;
//                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
//            }
            break;
        case BL_STEP_WAIT_FOR_REST:
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_go_app_timer))
            {
                bl_ps = BL_STEP_GOTO_APP;
                soft_timer_set(&s_bl_go_app_timer, BL_GO_APP_TIME);
            }
            else {}
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
        case BL_STEP_ERASE_MEMORY_PENDING:
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_common_timer))
            {
                bl_ps = BL_STEP_ECU_RESET;
                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            }
            else {}
            break;
        case BL_STEP_ERASE_MEMORY_REPEAT_PENDING:
            bl_ps = BL_STEP_ERASE_MEMORY;
            break;
        case BL_STEP_DOWNLOAD_FLASHDRIVER_PENDING:
            soft_timer_set(&s_bl_app_dw_timer, BL_APP_DW_TIME);
            break;
        case BL_STEP_DOWNLOAD_APP_PENDING:
        case BL_STEP_DOWNLOAD_INTEGRITY_PENDING:
            
        case BL_STEP_CHK_APP_DEPENDENCIES_PENDING:
            if (DEF_TRUE == is_soft_timer_timeout(&s_bl_app_dw_timer))
            {
                bl_ps = BL_STEP_ECU_RESET;
                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            }
            else {}
            break;
        default:
            break;
    }
    s_bl_process_step = bl_ps;
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

    (void)cbk_param2;
    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        if (DCM_SID10_SUB_01 == msg->req_data[1])
        {
            if (s_bl_info.app_valid_flg)
            {
                s_bl_process_step = BL_STEP_GOTO_APP;
                soft_timer_set(&s_bl_go_app_timer, BL_GO_APP_TIME);
                g_boot_app_chg_sts = BOOT_2_APP_1001;
                write_boot_app_chg_sts_2_e2(g_boot_app_chg_sts);
                msg->nrc = NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING;
            }
            else {}
        }
        else if (DCM_SID10_SUB_03 == msg->req_data[1])
        {
            /* nothing */
        }
        else if (DCM_SID10_SUB_02 == msg->req_data[1])
        {
            if (BL_STEP_SW_PROGRAMMING_SESSION_PENDING == s_bl_process_step)
            {
                s_bl_process_step = BL_STEP_SW_PROGRAMMING_SESSION;
            }
            else { /* nothing */ }
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
    uint32 temp = 0U;
    unUINT32_B tempb = {0U}; 

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        common_memcpy((uint08*)&s_bl_sid31_dp, (uint08*)cbk_param2, sizeof(stSID31_DP_INFO));
        #if (APP_CRC32_TYP == CRC32_TYP_AFTER_RID0202)
        if ((RID_CHECK_PROGRAMMING_INTEGRITY == s_bl_sid31_dp.req_rid) && (BL_STEP_DOWNLOAD_INTEGRITY_PENDING == s_bl_process_step))
        {
            temp = ((uint32)msg->req_data[4] << 24U) + 
                   ((uint32)msg->req_data[5] << 16U) +
                   ((uint32)msg->req_data[6] << 8U) +
                   ((uint32)msg->req_data[7] << 0U);
            s_bl_recv_crc = temp;
            msg->nrc = NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING;
            s_bl_info.mem_erase_nrc78_flg = 1;

        }
        else if ((RID_ERASE_MEMORY == s_bl_sid31_dp.req_rid)&&(BL_STEP_ERASE_MEMORY_PENDING == s_bl_process_step|| BL_STEP_CHK_APP_DEPENDENCIES_PENDING == s_bl_process_step))
        {
          msg->nrc = NRC_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING;
          s_bl_info.mem_erase_nrc78_flg = 1;
        }
        else if ((RID_CHECK_PROGRAMMING_DEPENDENCIES == s_bl_sid31_dp.req_rid)&&(BL_STEP_CHK_APP_DEPENDENCIES_PENDING == s_bl_process_step))//依赖性校验      (RID_CHECK_PROGRAMMING_DEPENDENCIES == s_bl_sid31_dp.req_rid)//
        {
             tempb.bytes = 0;
            if(APP_M0_A_START_ADDR == s_bl_info.recv_addr)
              bl_flash_read(APP_USER_M0_SPC_ADDR, (uint08*)&tempb, APP_USER_SPC_SIZE);//读取M0 APP依赖性信息
            else
              bl_flash_read(APP_USER_M4_SPC_ADDR, (uint08*)&tempb, APP_USER_SPC_SIZE);//读取M4 APP依赖性信息
            
            if (   (APP_USER_SPC_VAL == tempb.B.byte0)
                && (APP_USER_SPC_VAL == tempb.B.byte1)
                && (APP_USER_SPC_VAL == tempb.B.byte2)
                && (APP_USER_SPC_VAL == tempb.B.byte3)
                )
            {
//                s_bl_process_step = BL_STEP_ERASE_MEMORY_PENDING;//BL_STEP_WAIT_FOR_REST;
//                soft_timer_set(&s_bl_go_app_timer, BL_RESET_WAIT_TIME);
                
                
                  if(s_bl_info.recv_addr == APP_M0_A_START_ADDR)
                  {
//                      flash_flag_write(FLAG_M0_APP_ACTIVE, FLAG_VALID);   /* 写M0 APP有效标志位，初始化的时候会根据此标志位决定是否跳转到APP */   
                      
                      //这里为什么不直接进入BL_STEP_WAIT_FOR_REST状态的原因是：当上位机一键下载时，做完M0依赖性校验后会马上发M4擦除内存指令，所以要切到擦除内存等待状态。如果上位机是单独下载
                      //M0程序的时候，下一步就直接发11 01复位指令，也就直接复位了
                      s_bl_process_step = BL_STEP_ERASE_MEMORY_PENDING;//BL_STEP_WAIT_FOR_REST;
                      soft_timer_set(&s_bl_common_timer, BOOT_COMM_TIME);
                  }else if(s_bl_info.recv_addr == APP_M4_A_START_ADDR)
                  {
//                      flash_flag_write(FLAG_M4_APP_ACTIVE, FLAG_VALID);   /* 写M4 APP有效标志位，初始化的时候会根据此标志位决定是否跳转到APP */
                      
                      s_bl_process_step = BL_STEP_WAIT_FOR_REST;
                      soft_timer_set(&s_bl_go_app_timer, BL_RESET_WAIT_TIME);
                  }else
                  {
                  
                  }
            }
            else
            {
                msg->nrc = NRC_CONNDITIONS_NOT_ERROR;
                s_bl_process_step = BL_STEP_ECU_RESET;
                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            }
        }
        else
        {}
        #endif
    }
    else { /* nothing */ }
}

void bl_service_34_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    uint32 temp = 0;

    if (   (NRC_POSITIVE_RESPONSE == msg->nrc) 
        && ((BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step) || (BL_STEP_DOWNLOAD_FLASHDRIVER_PENDING == s_bl_process_step))
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
        if (BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step)
        {
            s_bl_info.recv_addr = s_bl_info.sid34_dp.mem_start_addr;
            s_bl_info.preprogram_addr = APP_PREPROGRAM_START_ADDR;
            if (s_bl_info.recv_addr < APP_M0_A_START_ADDR) /* protect boot code */
            {
                s_bl_process_step = BL_STEP_ECU_RESET;
                soft_timer_set(&s_bl_go_reset_timer, BL_GO_RESET_TIME);
            }
            else {}
            s_bl_init_crc = 0;
        }
        else
        {
            s_bl_info.recv_addr = (uint32)s_flash_driver_func;
        }

    }
    else { /* nothing */ }
}

void bl_service_36_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    #if (APP_CRC32_TYP == CRC32_TYP_FILE_TAIL)
    uint32 temp = 0;
    #endif

    if (   (NRC_POSITIVE_RESPONSE == msg->nrc) 
        && ((BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step) || (BL_STEP_DOWNLOAD_FLASHDRIVER_PENDING == s_bl_process_step))
        )
    {
        soft_timer_set(&s_bl_app_dw_timer, BL_APP_DW_TIME);
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
            if (BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step)
            {
                bl_flash_write(s_bl_info.preprogram_addr, (uint08*)s_flash_wrbuf, s_bl_info.tail_block_len);
                #if (APP_CRC32_TYP == CRC32_TYP_FILE_TAIL)
                s_bl_init_crc = CalcCrc32(s_bl_init_crc, s_flash_wrbuf, s_bl_info.tail_block_len - 4U);
                temp = ((uint32)s_flash_wrbuf[s_bl_info.tail_block_len - 4] << 24U) + 
               ((uint32)s_flash_wrbuf[s_bl_info.tail_block_len - 3] << 16U) +
               ((uint32)s_flash_wrbuf[s_bl_info.tail_block_len - 2] << 8U) +
               ((uint32)s_flash_wrbuf[s_bl_info.tail_block_len - 1] << 0U);
                s_bl_recv_crc = temp;
                #else
                s_bl_init_crc = CalcCrc32(s_bl_init_crc, s_flash_wrbuf, s_bl_info.tail_block_len);
                #endif
            }
            else
            {
                common_memcpy((uint08*)s_bl_info.preprogram_addr, (uint08*)s_flash_wrbuf, s_bl_info.tail_block_len);
            }
            s_bl_info.preprogram_addr += s_bl_info.tail_block_len;
            s_bl_info.sid36_dp.recv_over = 1;
        }
        else
        {
            
            if (BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step)
            {
                bl_flash_write(s_bl_info.preprogram_addr, (uint08*)s_flash_wrbuf, s_bl_info.block_len);
                s_bl_init_crc = CalcCrc32(s_bl_init_crc, s_flash_wrbuf, s_bl_info.block_len);
            }
            else
            {
                common_memcpy((uint08*)s_bl_info.preprogram_addr, (uint08*)s_flash_wrbuf, s_bl_info.block_len);
            }
            s_bl_info.preprogram_addr += s_bl_info.block_len;
        }       
        
        s_bl_info.pre_block_seq_cnt = s_bl_info.sid36_dp.block_seq_cnt;
        common_memcpy((uint08*)cbk_param2, (uint08*)&s_bl_info.sid36_dp, sizeof(stSID36_DP_INFO));
    }
    else { /* nothing */ }
}

void bl_service_37_cbk(void * cbk_param1, void * cbk_param2)
{
    stDCM_MSG_CONTEXT* msg = (stDCM_MSG_CONTEXT*)cbk_param1;
    uint32 temp = 0;
    
    (void)cbk_param2;

    if (NRC_POSITIVE_RESPONSE == msg->nrc)
    {
        if (BL_STEP_DOWNLOAD_APP_PENDING == s_bl_process_step)
        {
            #if (APP_CRC32_TYP == CRC32_TYP_AFTER_SID37)
            temp = ((uint32)msg->req_data[1] << 24U) + 
                   ((uint32)msg->req_data[2] << 16U) +
                   ((uint32)msg->req_data[3] << 8U) +
                   ((uint32)msg->req_data[4] << 0U);
            s_bl_recv_crc = temp;
            #endif
            s_bl_process_step = BL_STEP_DOWNLOAD_APP;

        }
        else if (BL_STEP_DOWNLOAD_FLASHDRIVER_PENDING == s_bl_process_step)
        {
            s_bl_process_step = BL_STEP_DOWNLOAD_FLASHDRIVER;
        }
        else { /* nothing */ }
    }
    else { /* nothing */ }
}

uint08 routine_erase_memory(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    
    if (s_flashdrive_flag && (BL_STEP_ERASE_MEMORY_PENDING == s_bl_process_step || BL_STEP_CHK_APP_DEPENDENCIES_PENDING == s_bl_process_step))
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

uint08 routine_check_programming_integrity(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    
    if (BL_STEP_DOWNLOAD_INTEGRITY_PENDING == s_bl_process_step)
    {
        s_bl_process_step = BL_STEP_DOWNLOAD_INTEGRITY;
        if (DEF_TRUE == chk_app_valid_crc())
        {
            nrc = NRC_POSITIVE_RESPONSE;
//            eeprom_drv_write(APP_BOOT_JUMP_FLAG_ADDR, (uint08*)&jump_flag, 4U); /* clear flag */
 
        }
        else
        {
            nrc = NRC_GENERAL_PROGRAMMING_FALIURE;
        }
    }
    else if (BL_STEP_DOWNLOAD_FLASHDRIVER == s_bl_process_step)
    {
        s_bl_process_step = BL_STEP_CHK_FLASHDRIVER_INTEGRITY;
        s_flashdrive_flag = 1;
    }
    else { /* nothing */ }

    return nrc;
}

uint08 routine_check_programming_pre_conditions(void)
{
    uint08 nrc = NRC_POSITIVE_RESPONSE;
    
    if (BL_STEP_CHK_PRE_PROG_CONDITION_PENDING == s_bl_process_step)
    {
        s_bl_process_step = BL_STEP_CHK_PRE_PROG_CONDITION;
    }
    else { /* nothing */ }

    return nrc;
}

void write_boot_app_chg_sts_2_e2(eBOOT_APP_CHG_STS sts)
{
    unUINT32_B tempb = {0U};
    
    tempb.bytes = sts;
    eeprom_drv_write(APP_BOOT_CHG_FLAG_ADDR, (uint08*)&tempb, 4U); /* write change status */
}

uint8_t t_pcan_data_send[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
void pcan_test_can_send(void)
{
    if ((BL_STEP_REQUEST_SEED == s_bl_process_step) || (BL_STEP_SW_PROGRAMMING_SESSION == s_bl_process_step))
    {
        can_transmit(0, 0x18181818, 8, t_pcan_data_send);
    }
}

void bl_test(void)
{
    (void)bl_app_valid_process();
}

