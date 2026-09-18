/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : bootloader.h
* Author        : yangming
* Date          : 2024-03-26
* Version       : 1.00
* Description   : UDS bootloader.
* Others        : None
*
****************************************************************************************************/
#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H
#include "platform_common_typdef.h"
#include "can_dcm.h"

/* pflash address */
#define BOOT_START_ADDR                                 CY_FLASH_LG_SBM_BASE
#define APP_M0_A_START_ADDR                             (CY_FLASH_LG_SBM_BASE+0x10000)
#define APP_M0_A_END_ADDR                               (CY_FLASH_LG_SBM_BASE+0x30000)
#define APP_M4_A_START_ADDR                             (CY_FLASH_LG_SBM_BASE+0x30000)
#define APP_M4_A_END_ADDR                               (CY_FLASH_LG_SBM_BASE+0x60000)

#define APP_PREPROGRAM_START_ADDR                       (CY_FLASH_LG_SBM_BASE+0x60000)
#define APP_PREPROGRAM_END_ADDR                         (CY_FLASH_LG_SBM_BASE+0x90000)
//#define APP_M4_B_START_ADDR                             (CY_FLASH_LG_SBM_BASE+0x60000)
//#define APP_M4_B_END_ADDR                               (CY_FLASH_LG_SBM_BASE+0x90000)

#define ECU_END_ADDR                                    (CY_FLASH_LG_SBM_BASE+0x90000)
#define APP_USER_M0_INFO_ADDR                           0x10010400
#define APP_USER_M4_INFO_ADDR                           0x10030500
#define APP_USER_M0_SPC_ADDR                            0x10010410
#define APP_USER_M4_SPC_ADDR                            0x10030510

//#define APP_A_VALID_ADDR                                0x0000B004
//#define APP_B_VALID_ADDR                                0x0000B008

/* eeprom address */
#define APP_UPDATE_FLAG_ADDR                            CY_WFLASH_SM_SBM_BASE + 0 * CY_WORK_SES_SIZE_IN_BYTE
#define APP_VALID_ADDR                                  CY_WFLASH_SM_SBM_BASE + 1 * CY_WORK_SES_SIZE_IN_BYTE
#define APP_BOOT_CHG_FLAG_ADDR                          CY_WFLASH_SM_SBM_BASE + 2 * CY_WORK_SES_SIZE_IN_BYTE
#define APP_BOOT_JUMP_FLAG_ADDR                         CY_WFLASH_SM_SBM_BASE + 3 * CY_WORK_SES_SIZE_IN_BYTE //app boot 交互标志位

#define APP_A_TOTAL_SIZE                                (ECU_END_ADDR + 1U - APP_A_START_ADDR)//(APP_A_END_ADDR + 1U - APP_A_START_ADDR)
#define APP_B_TOTAL_SIZE                                (APP_B_END_ADDR + 1U - APP_B_START_ADDR)
#define APP_USER_INFO_SIZE                              20U
#define APP_USER_SPC_SIZE                               4U

#define APP_USER_SPC_VAL                                0x55

#define APP_UPDATE_FLAG                                 0x00000001
#define APP_UPDATE_FLAG_TEST                            0x00000001
#define APP_UPDATE_FLAG_OTA                             0x00000002

#define TO_BOOT_FLAG                                    0x00000000
#define TO_APP_FLAG                                     0x00000001


//#define APP_VALID_VAL                                   0x20240122

#define BOOT_COMM_TIME                                  5000U
#define BL_APP_DW_TIME                                  120000U
#define BL_GO_APP_TIME                                  100U
#define BL_GO_RESET_TIME                                200U

#define CRC32_TYP_AFTER_SID37                           0 /* each file */
#define CRC32_TYP_AFTER_RID0202                         1 /* each file */
#define CRC32_TYP_AFTER_RIDF1A0                         2 /* all files */
#define CRC32_TYP_FILE_TAIL                             3 /* The crc32 code is at the end of the file */
#define APP_CRC32_TYP                                   CRC32_TYP_AFTER_RID0202//CRC32_TYP_FILE_TAIL
typedef enum _bl_process_step                           eBL_PROCESS_STEP;
typedef enum _boot_app_chg_sts                          eBOOT_APP_CHG_STS;
typedef struct _bl_info                                 stBL_INFO;

enum _bl_process_step
{
    BL_STEP_RESET = 0,
    BL_STEP_CHK_UPDATE_FLG,
    BL_STEP_GOTO_BOOT,
    BL_STEP_CHK_APP_VALID,
    BL_STEP_SW_EXTENDED_SESSION,
    BL_STEP_CHK_PRE_PROG_CONDITION,
    BL_STEP_STOP_DTC,
    BL_STEP_BLOCK_USELESS_COM,
    BL_STEP_READ_VERSION,
    BL_STEP_SW_PROGRAMMING_SESSION,
    BL_STEP_REQUEST_SEED,
    BL_STEP_RESPONSE_KEY,
    BL_STEP_WRITING_FINGERPRINT,
    BL_STEP_DOWNLOAD_FLASHDRIVER,
    BL_STEP_CHK_FLASHDRIVER_DEPENDENCIES,
    BL_STEP_ERASE_MEMORY,
    BL_STEP_DOWNLOAD_APP,
    BL_STEP_CHK_APP_DEPENDENCIES,
    BL_STEP_GOTO_APP,
    BL_STEP_ECU_RESET,
    BL_STEP_CHK_PRE_PROG_CONDITION_PENDING,
    BL_STEP_CHK_PRE_PROG_CONDITION_REPEAT_PENDING,
    BL_STEP_DOWNLOAD_FLASHDRIVER_REPEAT_PENDING,
    BL_STEP_ERASE_MEMORY_PENDING,
    BL_STEP_ERASE_MEMORY_REPEAT_PENDING,
    BL_STEP_DOWNLOAD_APP_PENDING,
    BL_STEP_CHK_APP_DEPENDENCIES_PENDING,
    BL_STEP_WAIT_1002_RES_PENDING,

    BL_STEP_MAX
};


enum _boot_app_chg_sts
{
    BOOT_APP_CHG_INIT = 0,
    BOOT_2_APP_1101,
    BOOT_2_APP_1001,
    BOOT_2_APP_S3_TIMEOUT,
    APP_2_BOOT_1101,
    APP_2_BOOT_1002,

    BOOT_APP_CHG_MAX
};

struct _bl_dependencies
{
    uint08                  local_sw_ver_h;
    uint08                  local_sw_ver_m;
    uint08                  local_sw_ver_l;
    uint08                  local_hw_ver_h;
    uint08                  local_hw_ver_m;
    uint08                  local_hw_ver_l;
    uint08                  local_build_date_year;
    uint08                  local_build_date_month;
    uint08                  local_build_date_day;
    uint08                  local_boot_ver_h;
    uint08                  local_boot_ver_m;
    uint08                  local_boot_ver_l;
};

struct _bl_info
{
//    stBL_DEPENDENCIES       dependencies;
    stSID34_DP_INFO         sid34_dp;
    stSID36_DP_INFO         sid36_dp;
    uint32                  recv_addr;
    uint32                  block_cnt;
    uint32                  block_total;
    uint32                  block_len;
    uint32                  tail_block_len;
    uint08                  pre_block_seq_cnt;
    uint08                  update_flg;
    uint08                  app_valid_flg;
    uint08                  mem_erase_nrc78_flg;
    PDU_ID                  mem_erase_hrh;
};

extern eBOOT_APP_CHG_STS g_boot_app_chg_sts;

extern void bootloader_init(void);
extern void bootloader_main_process(void);
extern void pcan_test_can_send(void);
extern typ_bool chk_app_valid_crc(void);
extern void bl_test(void);
extern void write_boot_app_chg_sts_2_e2(eBOOT_APP_CHG_STS sts);

#endif /* _BOOTLOADER_H */

