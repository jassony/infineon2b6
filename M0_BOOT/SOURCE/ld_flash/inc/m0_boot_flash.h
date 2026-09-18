/*
 * m0_flash.h
 *
 *  Created on: 2026Äê3ÔÂ10ÈÕ
 *      Author: hzldy
 */

#ifndef M0_BSW_SOURCE_LD_FLASH_INC_M0_FLASH_H_
#define M0_BSW_SOURCE_LD_FLASH_INC_M0_FLASH_H_


#include "cy_project.h"
#include "cy_device_headers.h"

#define FLASH_WRITE_MAX_SIZE 512


void Flash_Init(void);
void Flash_Erase(uint32_t addr,uint32_t size);
void Flash_Write(uint32_t addr,uint8_t * data,uint32_t size);
bool Flash_Task_Complete(void);
void Work_Flash_Erase(uint32_t addr,uint32_t size);
void Work_Flash_Write(uint32_t addr,uint8_t * data,uint32_t size);
void Work_Flash_Read(uint32_t addr,uint8_t * data,uint32_t size);

#endif /* M0_BSW_SOURCE_LD_FLASH_INC_M0_FLASH_H_ */
