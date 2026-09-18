/*
 * m0_flash.c
 *
 *  Created on: 2026年3月10日
 *      Author: hzldy
 */


#include "m0_boot_flash.h"
static bool completeflag   = false;
static bool nb_modeenabled = false;
static void Flash_Isr(void);
static uint8_t s_flash_buf[FLASH_WRITE_MAX_SIZE];
void Flash_Init(void)
{
  Cy_Srom_SetResponseHandler(Flash_Isr, CPUIntIdx5_IRQn);
  NVIC_SetPriority(CPUIntIdx5_IRQn, 3ul);
  NVIC_EnableIRQ(CPUIntIdx5_IRQn);
  Cy_Flashc_MainWriteEnable();
  Cy_Flashc_WorkWriteEnable();
  nb_modeenabled = true;
  completeflag = true;

}


void Flash_Erase(uint32_t addr,uint32_t size)
{
    cy_stc_flash_erasesector_config_t eraseSectorConfig = {0};
    uint32_t sector_size;
    sector_size = Cy_Flash_IsMainSmallSector(addr) ? CY_CODE_SES_SIZE_IN_BYTE : CY_CODE_LES_SIZE_IN_BYTE;
    for(uint32_t i = addr; i < addr + size; i += sector_size)
    {
      completeflag = false;
      eraseSectorConfig.blocking = CY_FLASH_ERASESECTOR_BLOCKING;
      eraseSectorConfig.intrMask = CY_FLASH_ERASESECTOR_NOT_SET_INTR_MASK;
      eraseSectorConfig.Addr = (uint32_t*)i;
      Cy_Flash_EraseSector(NULL, &eraseSectorConfig, CY_FLASH_DRIVER_NON_BLOCKING);
      while(!Flash_Task_Complete()) ; 
      sector_size = Cy_Flash_IsMainSmallSector(i+sector_size) ? CY_CODE_SES_SIZE_IN_BYTE : CY_CODE_LES_SIZE_IN_BYTE;

    }


}
void Flash_Write(uint32_t addr,uint8_t * data,uint32_t size)
{
    cy_stc_flash_programrow_config_t programRowConfig = {0};
    uint32_t sector_size;
    sector_size = Cy_Flash_IsMainSmallSector(addr) ? 8 : 512;
    for(uint32_t i = addr; i < addr + size; i += sector_size)
    {
      completeflag = false;
      memset(s_flash_buf,0xff,sector_size);
      if(sector_size + i > addr + size)
        memcpy(s_flash_buf,(uint32_t*)(data + (i - addr)),  addr + size - i);
      else
        memcpy(s_flash_buf,(uint32_t*)(data + (i - addr)),sector_size);
      programRowConfig.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
      programRowConfig.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
      programRowConfig.dataSize = sector_size == 8 ? CY_FLASH_PROGRAMROW_DATA_SIZE_64BIT : CY_FLASH_PROGRAMROW_DATA_SIZE_4096BIT;
      programRowConfig.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
      programRowConfig.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;
      programRowConfig.destAddr = (uint32_t*)i;
      programRowConfig.dataAddr = (uint32_t*)s_flash_buf; 
      Cy_Flash_ProgramRow(NULL, &programRowConfig, CY_FLASH_DRIVER_NON_BLOCKING);
      while(!Flash_Task_Complete()) ; 

    }

}


bool Flash_Task_Complete(void)
{
  return completeflag;
}

static void Flash_Isr(void)
{
    un_srom_api_resps_t apiResp;
    cy_en_srom_api_status_t sromDrvStatus = Cy_Srom_GetApiResponse(&apiResp);
    if(sromDrvStatus == CY_SROM_STATUS_SUCCESS)
    {
//        CY_ASSERT(false);
      completeflag = true;

    }

    Cy_Flashc_InvalidateFlashCacheBuffer();


}



void Work_Flash_Erase(uint32_t addr,uint32_t size)
{
    cy_stc_flash_erasesector_config_t eraseSectorConfig = {0};
    uint32_t sector_size;
    sector_size = Cy_Flash_IsWorkSmallSector(addr) ? CY_WORK_SES_SIZE_IN_BYTE : CY_WORK_LES_SIZE_IN_BYTE;
    for(uint32_t i = addr; i < addr + size; i += sector_size)
    {
      completeflag = false;
      eraseSectorConfig.blocking = CY_FLASH_ERASESECTOR_BLOCKING;
      eraseSectorConfig.intrMask = CY_FLASH_ERASESECTOR_NOT_SET_INTR_MASK;
      eraseSectorConfig.Addr = (uint32_t*)i;
      Cy_Flash_EraseSector(NULL, &eraseSectorConfig, CY_FLASH_DRIVER_NON_BLOCKING);
      while(!Flash_Task_Complete()) ; 

    }


}

void Work_Flash_Write(uint32_t addr,uint8_t * data,uint32_t size)
{
    cy_stc_flash_programrow_config_t programRowConfig = {0};
    uint32_t sector_size;
    sector_size = 4;
    for(uint32_t i = addr; i < addr + size; i += sector_size)
    {
      completeflag = false;
      programRowConfig.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
      programRowConfig.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
      programRowConfig.dataSize = CY_FLASH_PROGRAMROW_DATA_SIZE_32BIT;
      programRowConfig.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
      programRowConfig.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;
      programRowConfig.destAddr = (uint32_t*)i;
      programRowConfig.dataAddr = (uint32_t*)(data + (i - addr)); 
      Cy_Flash_ProgramRow(NULL, &programRowConfig, CY_FLASH_DRIVER_NON_BLOCKING);
      while(!Flash_Task_Complete()) ; 

    }

}

void Work_Flash_Read(uint32_t addr,uint8_t * data,uint32_t size)
{
   cy_stc_flash_blankcheck_config_t blankCheckConfig;

    blankCheckConfig.addrToBeChecked       = (uint32_t*)addr,
    blankCheckConfig.numOfWordsToBeChecked = Cy_Flash_IsWorkSmallSector(addr)
                                   ? CY_WORK_SES_SIZE_IN_WORD
                                   : CY_WORK_LES_SIZE_IN_WORD;
    if(Cy_Flash_BlankCheck(NULL, &blankCheckConfig, CY_FLASH_DRIVER_BLOCKING))
    {
      memcpy((uint8_t*)data, (uint8_t*)addr, size);

    }
    else
    {
      memset((uint8_t*)data, 0xff, size);

    }

    



}

//uint8_t flash_test[4096];

void Flash_Test(void)
{
//    memset(flash_test,0x55,4096);
//    Flash_Erase(0x10010000,4096);
//    Flash_Erase(CY_FLASH_SM_SBM_END - CY_CODE_SES_SIZE_IN_BYTE,4096);
//    Flash_Write(0x10010000,(uint32_t *)flash_test,4096);
//    Flash_Write(CY_FLASH_SM_SBM_END - CY_CODE_SES_SIZE_IN_BYTE,(uint32_t *)flash_test,2048);
//
//    Work_Flash_Erase(CY_WFLASH_LG_SBM_END - CY_WORK_LES_SIZE_IN_BYTE,2048);
//    Work_Flash_Erase(CY_WFLASH_SM_SBM_END - 16*CY_WORK_SES_SIZE_IN_BYTE,2048);
//    Work_Flash_Write(CY_WFLASH_LG_SBM_END - CY_WORK_LES_SIZE_IN_BYTE,(uint32_t *)flash_test,2048);
//    Work_Flash_Write(CY_WFLASH_SM_SBM_END - 16*CY_WORK_SES_SIZE_IN_BYTE,(uint32_t *)flash_test,2048);
//



}











