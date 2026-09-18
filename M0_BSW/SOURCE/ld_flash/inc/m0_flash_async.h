/*
 * Asynchronous flash request engine used by m0_flash.
 */
#ifndef M0_BSW_SOURCE_LD_FLASH_INC_M0_FLASH_ASYNC_H_
#define M0_BSW_SOURCE_LD_FLASH_INC_M0_FLASH_ASYNC_H_

#include "cy_project.h"
#include "cy_device_headers.h"

#define FLASH_TASK_QUEUE_DEPTH   (16u)
#define FLASH_DATA_BUF_SIZE      (4096u)
#define FLASH_TASK_TIMEOUT_MS    (5000u)

void Flash_Async_Init(void);
void Flash_Async_QueueErase(uint8_t mainFlash, uint32_t addr, uint32_t size);
void Flash_Async_QueueWrite(uint8_t mainFlash, uint32_t addr,
                            const uint8_t *data, uint32_t size);
void Flash_Async_Read(uint32_t addr, uint8_t *data, uint32_t size);
void Flash_Async_Process(void);
void Flash_Async_Wait(void);
bool Flash_Async_IsIdle(void);

#endif /* M0_BSW_SOURCE_LD_FLASH_INC_M0_FLASH_ASYNC_H_ */
