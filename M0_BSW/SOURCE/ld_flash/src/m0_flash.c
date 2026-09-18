/*
 * m0_flash.c
 *
 *  Created on: 2026年3月10日
 *      Author: hzldy
 *
 * Public interface to the asynchronous flash engine.
 *
 * The original blocking implementation spun in "while(!Flash_Task_Complete());"
 * loops. All erase / program work has been moved into m0_flash_async.c and is
 * now executed asynchronously:
 *   - these entry points only queue the request (address / size / data) and
 *     return immediately,
 *   - Flash_Task_Process() must be called from the main while(1) loop; it drives
 *     every erase / write operation and waits on the SROM completion flag.
 */


#include "m0_flash.h"
#include "m0_flash_async.h"


void Flash_Init(void)
{
    Flash_Async_Init();
}

void Flash_Erase(uint32_t addr, uint32_t size)
{
    Flash_Async_QueueErase(1u, addr, size);
}

void Flash_Write(uint32_t addr, uint8_t * data, uint32_t size)
{
    Flash_Async_QueueWrite(1u, addr, data, size);
}

bool Flash_Task_Complete(void)
{
    return Flash_Async_IsIdle();
}

void Flash_Task_Process(void)
{
    Flash_Async_Process();
}

void Flash_Task_Wait(void)
{
    Flash_Async_Wait();
}

/*
 * Synchronous (blocking) variants.
 * The request is queued at the tail of the same engine queue and then waited on
 * until the whole queue is idle, so a synchronous call never interrupts / jumps
 * ahead of an asynchronous operation that is already running. Only use these
 * where the caller really must know the flash access finished before continuing
 * (e.g. bootloader write/erase or just before a reset).
 */
void Flash_Erase_Sync(uint32_t addr, uint32_t size)
{
    Flash_Async_QueueErase(1u, addr, size);
    Flash_Async_Wait();
}

void Flash_Write_Sync(uint32_t addr, uint8_t * data, uint32_t size)
{
    Flash_Async_QueueWrite(1u, addr, data, size);
    Flash_Async_Wait();
}

void Work_Flash_Erase_Sync(uint32_t addr, uint32_t size)
{
    Flash_Async_QueueErase(0u, addr, size);
    Flash_Async_Wait();
}

void Work_Flash_Write_Sync(uint32_t addr, uint8_t * data, uint32_t size)
{
    Flash_Async_QueueWrite(0u, addr, data, size);
    Flash_Async_Wait();
}

void Work_Flash_Erase(uint32_t addr, uint32_t size)
{
    Flash_Async_QueueErase(0u, addr, size);
}

void Work_Flash_Write(uint32_t addr, uint8_t * data, uint32_t size)
{
    Flash_Async_QueueWrite(0u, addr, data, size);
}

void Work_Flash_Read(uint32_t addr, uint8_t * data, uint32_t size)
{
    Flash_Async_Read(addr, data, size);
}

/*
 * Manual async write test hook (extern declared in m0_task.c).
 * Queues, for each of the four test addresses, one erase plus two 128-byte
 * writes of 0x55, then returns immediately -- the main while(1) loop's
 * Flash_Task_Process() drives all erase / program operations asynchronously.
 *
 *   main  flash : 0x10060000, 0x10070000   (one sector, both groups inside)
 *   work  flash : 0x1400b000 (LG/LES), 0x1400d000 (SM/SES, groups cross two
 *               128-byte sectors, so the erase covers 256 bytes)
 */
void Flash_Test(void)
{
    uint8_t  data[128];
    static const uint32_t test_addr[4u] = { 0x10060000u, 0x10070000u,
                                            0x1400b000u, 0x1400d000u };
    uint32_t i;

    for (i = 0u; i < sizeof(data); i++)
    {
        data[i] = 0x55u;
    }

    for (i = 0u; i < 4u; i++)
    {
        uint32_t addr = test_addr[i];

        if (addr >= CY_WFLASH_LG_SBM_BASE)
        {
            Work_Flash_Erase(addr, 256u);              /* covers both groups  */
            Work_Flash_Write(addr, data, 128u);        /* group 1             */
            Work_Flash_Write(addr + 128u, data, 128u); /* group 2             */
        }
        else
        {
            Flash_Erase(addr, 256u);                   /* covers both groups  */
            Flash_Write(addr, data, 128u);             /* group 1             */
            Flash_Write(addr + 128u, data, 128u);      /* group 2             */
        }
    }
}

