/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_driver.c
* Author        : yangming
* Date          : 2024-03-21
* Version       : 1.00
* Description   : EEPROM driver.
* Others        : None
*
*******************************************************************************************************/
#include "eeprom_driver.h"
#include "m0_boot_flash.h"


//static flash_ssd_config_t s_ee_flash_cfg;
static typ_bool s_ee_drv_wr_sts;
static typ_bool s_ee_drv_rd_sts;
static uint8_t s_work_flash_buf[CY_WORK_LES_SIZE_IN_BYTE];

static void s32k144_eeprom_init(void)
{
//    status_t ret = STATUS_SUCCESS;
//
//    INT_SYS_DisableIRQGlobal();
//	ret = FLASH_DRV_Init(&Flash1_InitConfig0, &s_ee_flash_cfg);
//   	DEV_ASSERT(STATUS_SUCCESS == ret);
//
//   if (s_ee_flash_cfg.EEESize == 0u) //检查FlexRAM是否已配置为EEPROM，为0表示目前是传统RAM
//   {
//		/*
//		* 将 FlexRAM 配置为 EEPROM，将 FlexNVM 配置为 EEPROM 备份区域，
//		* EEEDataSizeCode = 0x02u：EEPROM 大小 = 4 KB
//		* DEPartitionCode = 0x08u：EEPROM 备份大小 = 64 KB */
//       ret = FLASH_DRV_DEFlashPartition(&s_ee_flash_cfg, 0x02u, 0x08u, 0x0u, false, true); //分区
//       DEV_ASSERT(STATUS_SUCCESS == ret);
//
//       /* Re-initialize the driver to update the new EEPROM configuration */
//       ret = FLASH_DRV_Init(&Flash1_InitConfig0,&s_ee_flash_cfg);
//       DEV_ASSERT(STATUS_SUCCESS == ret);
//
//       ret = FLASH_DRV_SetFlexRamFunction(&s_ee_flash_cfg, EEE_ENABLE, 0x00u, NULL);
//       DEV_ASSERT(STATUS_SUCCESS == ret);
//   }
//   else    /* FLexRAM is already configured as EEPROM */
//   {
//       ret = FLASH_DRV_SetFlexRamFunction(&s_ee_flash_cfg, EEE_ENABLE, 0x00u, NULL);
//       DEV_ASSERT(STATUS_SUCCESS == ret);
//   }
//   INT_SYS_EnableIRQGlobal();
}
void common_memcpy_u32(uint32* dest, uint32* src, uint32 length)
{
    if ((DEF_NULL == dest) || (DEF_NULL == src) || (0U == length))
    {
        return;
    }

    while (length--)
    {
        *dest = *src;
        dest++;
        src++;
    }
}
static typ_bool s32k144_eeprom_write(uint32 eeprom_addr, uint08* data_buf, uint32 length)
{
	uint32 eepromaddress = eeprom_addr; //起始地址
	uint32 eepromsize = length;     //超出长度
//	status_t ret = STATUS_SUCCESS;
        uint16_t exceed;
        if(eeprom_addr >= CY_WFLASH_LG_SBM_BASE && eeprom_addr < CY_WFLASH_SM_SBM_BASE)
        {
          exceed = eepromaddress % CY_WORK_LES_SIZE_IN_BYTE;
          eepromaddress -= exceed;
          if(exceed + length > CY_WORK_LES_SIZE_IN_BYTE)
          {
            while(eeprom_addr  > (CY_WFLASH_SM_SBM_BASE - CY_WORK_LES_SIZE_IN_BYTE));
            
            eepromsize -= (exceed + eepromsize) % CY_WORK_LES_SIZE_IN_BYTE;
            Work_Flash_Read(eepromaddress,s_work_flash_buf,CY_WORK_LES_SIZE_IN_BYTE);
//            common_memcpy((uint08*)work_flash_buf, (uint08*)eepromaddress, CY_WORK_LES_SIZE_IN_BYTE);
//            work_flash_buf[0] = *(uint32_t*)eepromaddress;
            common_memcpy((uint08*)(s_work_flash_buf + exceed),data_buf, eepromsize);
            Work_Flash_Erase(eepromaddress,CY_WORK_LES_SIZE_IN_BYTE);
            Work_Flash_Write(eepromaddress,s_work_flash_buf,CY_WORK_LES_SIZE_IN_BYTE);
            
            Work_Flash_Read(eepromaddress + CY_WORK_LES_SIZE_IN_BYTE,s_work_flash_buf,CY_WORK_LES_SIZE_IN_BYTE);
            common_memcpy((uint08*)s_work_flash_buf,data_buf + (CY_WORK_LES_SIZE_IN_BYTE - exceed), length - eepromsize);
            Work_Flash_Erase(eepromaddress + CY_WORK_LES_SIZE_IN_BYTE,CY_WORK_LES_SIZE_IN_BYTE);
            Work_Flash_Write(eepromaddress + CY_WORK_LES_SIZE_IN_BYTE,s_work_flash_buf,CY_WORK_LES_SIZE_IN_BYTE);
          }
          else
          {
            Work_Flash_Read(eepromaddress,s_work_flash_buf,CY_WORK_LES_SIZE_IN_BYTE);
//            common_memcpy((uint08*)work_flash_buf, (uint08*)eepromaddress, CY_WORK_LES_SIZE_IN_BYTE);
            common_memcpy((uint08*)(s_work_flash_buf + exceed),data_buf, eepromsize);
            Work_Flash_Erase(eepromaddress,CY_WORK_LES_SIZE_IN_BYTE);
            Work_Flash_Write(eepromaddress,s_work_flash_buf,CY_WORK_LES_SIZE_IN_BYTE);
          }
        
        }
        else if(eeprom_addr >= CY_WFLASH_SM_SBM_BASE && eeprom_addr < (CY_WFLASH_SM_SBM_BASE + CY_WFLASH_SM_SBM_SIZE))
        {
          exceed = eepromaddress % CY_WORK_SES_SIZE_IN_BYTE;
          eepromaddress -= exceed;
          if(exceed + length > CY_WORK_SES_SIZE_IN_BYTE)
          {
            Work_Flash_Read(eepromaddress,s_work_flash_buf,CY_WORK_SES_SIZE_IN_BYTE);
            Work_Flash_Read(eepromaddress+CY_WORK_SES_SIZE_IN_BYTE,s_work_flash_buf+CY_WORK_SES_SIZE_IN_BYTE,CY_WORK_SES_SIZE_IN_BYTE);
//            common_memcpy((uint08*)work_flash_buf, (uint08*)eepromaddress, CY_WORK_SES_SIZE_IN_BYTE*2);
            common_memcpy((uint08*)(s_work_flash_buf + exceed),data_buf, length);
            Work_Flash_Erase(eepromaddress,CY_WORK_SES_SIZE_IN_BYTE*2);
            Work_Flash_Write(eepromaddress,s_work_flash_buf,CY_WORK_SES_SIZE_IN_BYTE*2);
          }
          else
          {
            Work_Flash_Read(eepromaddress,s_work_flash_buf,CY_WORK_SES_SIZE_IN_BYTE);
//            common_memcpy((uint08*)work_flash_buf, (uint08*)eepromaddress, CY_WORK_SES_SIZE_IN_BYTE);
            common_memcpy((uint08*)(s_work_flash_buf + exceed),data_buf, length);
            Work_Flash_Erase(eepromaddress,CY_WORK_SES_SIZE_IN_BYTE);
            Work_Flash_Write(eepromaddress,s_work_flash_buf,CY_WORK_SES_SIZE_IN_BYTE);
          }
        
        }
        else
        {
          while(1);
          
        }
//	ret = FLASH_DRV_EEEWrite(&s_ee_flash_cfg, eepromaddress, eepromsize, data_buf);
//	if (STATUS_SUCCESS == ret)
//	{
        return DEF_TRUE;
//	}
//	else
//	{
//        return DEF_FALSE;
//	}
  
}

static typ_bool s32k144_eeprom_read(uint32 eeprom_addr, uint08* data_buf, uint32 length)
{
    uint32 eepromaddress = eeprom_addr;

//    common_memcpy((uint08*)data_buf, (uint08*)eepromaddress, length);
    Work_Flash_Read(eepromaddress,data_buf,length);

    return DEF_TRUE;
}

void eeprom_drv_init(void)
{
//    m95_init();
    s32k144_eeprom_init();
}

typ_bool s32k144_flash_read(uint32 addr, uint08* data, uint32 len)
{
    common_memcpy((uint08*)data, (uint08*)addr, len);

    return DEF_TRUE;
}

typ_bool s32k144_flash_write(uint32 addr, uint08* data, uint32 len)
{
    typ_bool ret = DEF_TRUE;
//    status_t sts = STATUS_ERROR;
//    
//    INT_SYS_DisableIRQGlobal();
//    sts = FLASH_DRV_Program(&s_ee_flash_cfg, addr, len, data);
//    INT_SYS_EnableIRQGlobal();
//    if (STATUS_SUCCESS == sts)
//    {
//        ret = DEF_TRUE;
//    }
//    else
//    {
//        ret = DEF_FALSE;
//    }
    Flash_Write(addr,data,len);

    return ret;
}

typ_bool s32k144_flash_erase_sector(uint32 addr, uint32 size)
{
    typ_bool ret = DEF_TRUE;
//    status_t sts = STATUS_ERROR;
//    
//    INT_SYS_DisableIRQGlobal();
//    sts = FLASH_DRV_EraseSector(&s_ee_flash_cfg, addr, size);
//    INT_SYS_EnableIRQGlobal();
//    if (STATUS_SUCCESS == sts)
//    {
//        ret = DEF_TRUE;
//    }
//    else
//    {
//        ret = DEF_FALSE;
//    }
    Flash_Erase(addr,size);

    return ret;
}

void eeprom_drv_write(uint32 addr, uint08* data, uint32 len)
{
    s_ee_drv_wr_sts = s32k144_eeprom_write(addr, data, len);
}

void eeprom_drv_read(uint32 addr, uint08* data, uint32 len)
{
    s_ee_drv_rd_sts = s32k144_eeprom_read(addr, data, len);
}

typ_bool eeprom_drv_wr_sts_get(void)
{
    return s_ee_drv_wr_sts;   
}

typ_bool eeprom_drv_rd_sts_get(void)
{
    return s_ee_drv_rd_sts;
}



#ifdef EEPROM_DRV_DEBUG_ENABLE
#define T_EE_COMMON_LEN         16U
uint08 t_ee_once = 0;
uint08 t_ee_addr = 0;
uint16 t_ee_len = T_EE_COMMON_LEN;
uint08 t_ee_wr_buf[T_EE_COMMON_LEN];
uint08 t_ee_rd_buf[T_EE_COMMON_LEN];
//extern void m95xxx_test(void);
#if 0
void eeprom_drv_test(void)
{
    common_memset((uint08*)t_ee_wr_buf, 0xAA, t_ee_len);
    common_memset((uint08*)t_ee_rd_buf, 0x55, t_ee_len);
    if (0 == t_ee_once)
    {
        t_ee_once = 0;        
        m95_mem_write(t_ee_addr, (uint08*)t_ee_wr_buf, t_ee_len);
        m95_mem_read(t_ee_addr, (uint08*)t_ee_rd_buf, t_ee_len);
//        m95xxx_test();
    }
}
#else
void eeprom_drv_test(void)
{
    t_ee_len = 16;
    common_memset((uint08*)t_ee_wr_buf, 0xAA, t_ee_len);
    common_memset((uint08*)t_ee_rd_buf, 0x55, t_ee_len);
//    if (0 == t_ee_once)
//    {
//        t_ee_once = 1;
    s32k144_eeprom_write(0x1400bf00, t_ee_wr_buf, t_ee_len);
    s32k144_eeprom_read(0x1400bf00, t_ee_rd_buf, t_ee_len);
    common_memset((uint08*)t_ee_rd_buf, 0x55, t_ee_len);
    s32k144_eeprom_write(0x140007ff, t_ee_wr_buf, t_ee_len);
    s32k144_eeprom_read(0x140007ff, t_ee_rd_buf, t_ee_len);
    common_memset((uint08*)t_ee_rd_buf, 0x55, t_ee_len);
    s32k144_eeprom_write(0x1400c700, t_ee_wr_buf, t_ee_len);
    s32k144_eeprom_read(0x1400c700, t_ee_rd_buf, t_ee_len);

    common_memset((uint08*)t_ee_rd_buf, 0x55, t_ee_len);
    s32k144_eeprom_write(0x1400c77f, t_ee_wr_buf, t_ee_len);
    s32k144_eeprom_read(0x1400c77f, t_ee_rd_buf, t_ee_len);
//      common_memset((uint08*)t_ee_rd_buf, 0x55, t_ee_len);

//    }
}
#endif
#endif

