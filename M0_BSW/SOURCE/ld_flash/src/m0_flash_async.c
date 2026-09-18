/*
 * m0_flash_async.c
 *
 *  Created on: 2026年8月22日
 *      Author: hzldy
 *
 * Asynchronous (non-blocking) flash write / erase engine.
 *
 * Design:
 *   - The queue functions (Flash_Async_QueueErase / Flash_Async_QueueWrite)
 *     only record the request and copy write data into an internal data
 *     buffer, then return. All Cy_Flash_* erase / program calls are deferred
 *     to Flash_Async_Process().
 *   - Write data is held in ONE append-allocated data buffer (s_data_buf[]):
 *     each queued write task owns a segment [dataOff, dataOff+dataLen)
 *     allocated from the tail, so many small write tasks can be queued without
 *     wasting whole fixed-size slots. A segment is reclaimed only when its
 *     task is popped, and tasks are processed strictly in FIFO order, so the
 *     data currently being programmed by the flash macro (the head task's
 *     segment) is never moved or overwritten by a later append.
 *   - Flash_Async_Process() must be called from the main while(1) loop. It
 *     walks the queue one flash chunk at a time. Each chunk is started as a
 *     CY_FLASH_DRIVER_NON_BLOCKING operation; the SROM interrupt handler
 *     (Flash_Isr) sets the complete flag when the chunk has finished. The state
 *     machine checks that flag before starting the next chunk, so every erase /
 *     program operation is executed asynchronously without busy-waiting in the
 *     calling code.
 */

#include "m0_flash_async.h"
#include "soft_timer.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Constants / private types                                          */
/* ------------------------------------------------------------------ */
#define FLASH_ROW_BUF_SIZE       (512u) /* main-flash program row (large sector)  */
#define FLASH_DATA_INVALID       (0xFFFFFFFFu) /* dataOff value when no segment is owned (erase) */

typedef enum
{
    FLASH_TASK_TYPE_ERASE = 0u,
    FLASH_TASK_TYPE_WRITE = 1u
} flash_task_type_t;

/* One queued flash task. Write tasks own a segment of s_data_buf[] via
   dataOff / dataLen; erase tasks have dataOff == FLASH_DATA_INVALID. */
typedef struct
{
    uint8_t  type;       /* flash_task_type_t                          */
    uint8_t  mainFlash;  /* 1u = code/main flash, 0u = work flash      */
    uint8_t  reserved;   /* reserved (alignment)                       */
    uint32_t addr;       /* start address                              */
    uint32_t size;       /* total bytes to process                     */
    uint32_t done;       /* bytes already processed                    */
    uint32_t chunk;      /* size of the chunk currently in flight     */
    uint32_t start_tick; /* 1 ms tick when the current chunk was issued */
    uint32_t dataOff;    /* offset of this task's data in s_data_buf[]  */
    uint32_t dataLen;    /* aligned length of that segment (multiple of 4) */
} flash_task_t;

/* ------------------------------------------------------------------ */
/* Static variables                                                    */
/* ------------------------------------------------------------------ */
static flash_task_t s_flash_queue[FLASH_TASK_QUEUE_DEPTH];
static uint8_t      s_q_head  = 0u; /* index of the next task to process */
static uint8_t      s_q_tail  = 0u; /* index of the next free slot       */
static uint8_t      s_q_count = 0u; /* number of queued tasks            */

/* Single append-allocated write-data buffer. dataOff / s_data_start /
   s_data_alloc are byte offsets into it; the array is uint32_t so every
   segment stays 4-byte aligned for the flash driver. */
static uint32_t s_data_buf[FLASH_DATA_BUF_SIZE / 4u];
static uint32_t s_data_start = 0u; /* offset of the oldest live segment      */
static uint32_t s_data_alloc = 0u; /* next free append offset                */

static uint32_t s_row_buf[FLASH_ROW_BUF_SIZE / 4u]; /* staging buffer for main-flash rows */

static bool     s_completeflag = true;  /* set by Flash_Isr when a SROM op finished */
static bool     s_op_inflight  = false; /* a non-blocking op is currently running   */

/* ------------------------------------------------------------------ */
/* Local helpers                                                       */
/* ------------------------------------------------------------------ */
static void   Flash_Isr(void);
static void   flash_queue_commit(void);
static void   flash_queue_pop(void);
static void   flash_task_abandon(flash_task_t *t);
static void   flash_issue_chunk(flash_task_t *t);

static void flash_queue_commit(void)
{
    s_q_tail = (uint8_t)((s_q_tail + 1u) % FLASH_TASK_QUEUE_DEPTH);
    s_q_count++;
}

static void flash_queue_pop(void)
{
    s_q_head = (uint8_t)((s_q_head + 1u) % FLASH_TASK_QUEUE_DEPTH);
    s_q_count--;
}

/* Drop a task from the queue. For a write task this also reclaims its data
   segment. Because tasks are processed strictly in FIFO order, the head
   task's segment is always the oldest live segment, so reclaiming is just
   advancing s_data_start; when the buffer is empty it is rewound to 0. */
static void flash_task_abandon(flash_task_t *t)
{
    if (t->type == FLASH_TASK_TYPE_WRITE)
    {
        s_data_start = t->dataOff + t->dataLen;
        if (s_data_start == s_data_alloc)
        {
            s_data_start = 0u;
            s_data_alloc = 0u;
        }
    }
    flash_queue_pop();
}

/* Start one non-blocking chunk of the given task. */
static void flash_issue_chunk(flash_task_t *t)
{
    uint32_t curAddr = t->addr + t->done;
    uint32_t remain  = t->size - t->done;
    uint32_t chunk   = 0u;

    t->start_tick = (uint32_t)timer_get_ticks(); /* arm the per-chunk timeout */

    if (t->type == FLASH_TASK_TYPE_ERASE)
    {
        cy_stc_flash_erasesector_config_t eraseCfg = {0u};

        if (t->mainFlash != 0u)
        {
            chunk = Cy_Flash_IsMainSmallSector(curAddr) ? CY_CODE_SES_SIZE_IN_BYTE
                                                        : CY_CODE_LES_SIZE_IN_BYTE;
        }
        else
        {
            chunk = Cy_Flash_IsWorkSmallSector(curAddr) ? CY_WORK_SES_SIZE_IN_BYTE
                                                        : CY_WORK_LES_SIZE_IN_BYTE;
        }

        eraseCfg.blocking = CY_FLASH_ERASESECTOR_BLOCKING;
        eraseCfg.intrMask = CY_FLASH_ERASESECTOR_NOT_SET_INTR_MASK;
        eraseCfg.Addr     = (uint32_t*)curAddr;
        s_completeflag = false;
        Cy_Flash_EraseSector(NULL, &eraseCfg, CY_FLASH_DRIVER_NON_BLOCKING);
    }
    else /* FLASH_TASK_TYPE_WRITE */
    {
        cy_stc_flash_programrow_config_t rowCfg = {0u};

        rowCfg.blocking = CY_FLASH_PROGRAMROW_BLOCKING;
        rowCfg.skipBC   = CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK;
        rowCfg.dataLoc  = CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM;
        rowCfg.intrMask = CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK;

        if (t->mainFlash != 0u)
        {
            /* Main flash: 8-byte rows in small sectors, 512-byte rows in large.
               A ProgramRow always programs a whole row at a row-aligned
               destination address, so each chunk is limited to the remainder of
               the row that curAddr falls into, and the destination is rounded
               back down to the row start (the task's data goes into the row at
               rowOff). Without this, a write that does not start on a row
               boundary -- e.g. Flash_Test's second 128-byte group at 0x10060080,
               inside a 512-byte row -- is sent to the SROM with a misaligned
               destAddr and fails, so that data is never programmed. */
            uint32_t rowSize = Cy_Flash_IsMainSmallSector(curAddr) ? 8u : FLASH_ROW_BUF_SIZE;
            uint32_t rowOff  = curAddr & (rowSize - 1u);

            chunk = (remain < (rowSize - rowOff)) ? remain : (rowSize - rowOff);
            memset((uint8_t*)s_row_buf, 0xff, rowSize);
            memcpy((uint8_t*)s_row_buf + rowOff,
                   (uint8_t*)&s_data_buf[(t->dataOff + t->done) / 4u],
                   chunk);

            rowCfg.dataSize = (rowSize == 8u) ? CY_FLASH_PROGRAMROW_DATA_SIZE_64BIT
                                              : CY_FLASH_PROGRAMROW_DATA_SIZE_4096BIT;
            rowCfg.destAddr = (uint32_t*)(curAddr - rowOff); /* row-aligned */
            rowCfg.dataAddr = s_row_buf;
        }
        else
        {
            /* Work flash: one 32-bit word (4 bytes) per program row. The SROM
               reads the SRAM segment while the op runs; the segment is only
               reclaimed after the task is popped, so it stays intact. */
            chunk = 4u;
            rowCfg.dataSize = CY_FLASH_PROGRAMROW_DATA_SIZE_32BIT;
            rowCfg.destAddr = (uint32_t*)curAddr;
            rowCfg.dataAddr = &s_data_buf[(t->dataOff + t->done) / 4u];
        }

        s_completeflag = false;
        Cy_Flash_ProgramRow(NULL, &rowCfg, CY_FLASH_DRIVER_NON_BLOCKING);
    }

    t->chunk = chunk;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
void Flash_Async_Init(void)
{
    Cy_Srom_SetResponseHandler(Flash_Isr, CPUIntIdx5_IRQn);
    NVIC_SetPriority(CPUIntIdx5_IRQn, 3ul);
    NVIC_EnableIRQ(CPUIntIdx5_IRQn);
    Cy_Flashc_MainWriteEnable();
    Cy_Flashc_WorkWriteEnable();

    s_q_head        = 0u;
    s_q_tail        = 0u;
    s_q_count       = 0u;
    s_data_start    = 0u;
    s_data_alloc    = 0u;
    s_completeflag  = true;
    s_op_inflight   = false;
}

void Flash_Async_QueueErase(uint8_t mainFlash, uint32_t addr, uint32_t size)
{
    flash_task_t *t;

    if (size == 0u)
    {
        return;
    }

    /* Wait for a free slot, pumping the state machine so the queue drains
       (only needed if more tasks are queued than FLASH_TASK_QUEUE_DEPTH). */
    while (s_q_count >= FLASH_TASK_QUEUE_DEPTH)
    {
        Flash_Async_Process();
    }

    t = &s_flash_queue[s_q_tail];
    t->type      = FLASH_TASK_TYPE_ERASE;
    t->mainFlash = mainFlash;
    t->addr      = addr;
    t->size      = size;
    t->done      = 0u;
    t->chunk     = 0u;
    t->dataOff   = FLASH_DATA_INVALID; /* erase tasks own no data segment */
    t->dataLen   = 0u;
    flash_queue_commit();
}

void Flash_Async_QueueWrite(uint8_t mainFlash, uint32_t addr, const uint8_t *data, uint32_t size)
{
    uint32_t offset = 0u;

    while (size > 0u)
    {
        uint32_t seg = size;
        flash_task_t *t;

        if (seg > FLASH_DATA_BUF_SIZE)
        {
            seg = FLASH_DATA_BUF_SIZE; /* split oversized writes into multiple tasks */
        }

        while (s_q_count >= FLASH_TASK_QUEUE_DEPTH)
        {
            Flash_Async_Process();
        }
        /* Wait until the append-allocated buffer can hold this segment. The
           allocator only rewinds when the buffer is fully drained, so this
           throttles the caller until enough queued data has been flashed. */
        while ((FLASH_DATA_BUF_SIZE - s_data_alloc) < seg)
        {
            Flash_Async_Process();
        }

        t = &s_flash_queue[s_q_tail];
        t->type      = FLASH_TASK_TYPE_WRITE;
        t->mainFlash = mainFlash;
        t->addr      = addr + offset;
        t->size      = seg;
        t->done      = 0u;
        t->chunk     = 0u;
        t->dataOff   = s_data_alloc;
        t->dataLen   = (seg + 3u) & ~3u; /* keep segments 4-byte aligned */
        memcpy((uint8_t*)s_data_buf + s_data_alloc, data + offset, seg);
        s_data_alloc += t->dataLen;
        flash_queue_commit();

        offset += seg;
        size   -= seg;
    }
}

void Flash_Async_Read(uint32_t addr, uint8_t *data, uint32_t size)
{
    cy_stc_flash_blankcheck_config_t blankCheckConfig;
    uint32_t l_checksize = size,l_checkaddr = addr;
    cy_en_flashdrv_status_t result;
    /* A read must see committed data. The flash macro is shared with
       erase/program, so pump the state machine until every queued task has
       finished before touching the flash. This also keeps read-back
       verifications (eeprom_ctrl write-CHK state) from running ahead of the
       asynchronous writes they are checking. */
    while (s_q_count != 0u)
    {
        Flash_Async_Process();
    }
    uint32_t l_size_in_word = Cy_Flash_IsWorkSmallSector(addr)
                                             ? CY_WORK_SES_SIZE_IN_WORD
                                               : CY_WORK_LES_SIZE_IN_WORD;
    while(l_checksize)
    {
    blankCheckConfig.addrToBeChecked       = (uint32_t*)l_checkaddr;
    blankCheckConfig.numOfWordsToBeChecked = (l_checksize < l_size_in_word ? l_checksize : l_size_in_word);
    if (Cy_Flash_BlankCheck(NULL, &blankCheckConfig, CY_FLASH_DRIVER_BLOCKING) == 0) 
      break;
    l_checksize -= blankCheckConfig.numOfWordsToBeChecked;
    l_checkaddr += blankCheckConfig.numOfWordsToBeChecked;
    }

    if (l_checksize == 0)
    {
        memcpy((uint8_t*)data, (uint8_t*)addr, size);
    }
    else
    {
        memset((uint8_t*)data, 0xff, size);
    }
}

/*
 * Call from the main while(1) loop. Processes queued tasks one chunk at a time:
 *   - if an operation is in flight, wait for the complete flag (ISR),
 *   - otherwise start the next chunk, or pop the task when it is done and
 *     continue with the next queued task.
 */
void Flash_Async_Process(void)
{
    flash_task_t *t;

    if (s_q_count == 0u)
    {
        return;
    }

    t = &s_flash_queue[s_q_head];

    if (s_op_inflight)
    {
        if (s_completeflag == false)
        {
            /* The current chunk has not completed within FLASH_TASK_TIMEOUT_MS:
               the flash operation is considered hung. Drop this task (releasing
               its data segment) and move on; the next Process call starts the
               next queued task. */
            if ((uint32_t)(timer_get_ticks() - t->start_tick) >= FLASH_TASK_TIMEOUT_MS)
            {
                s_op_inflight  = false;
                s_completeflag = true;
                flash_task_abandon(t);
                return;
            }
            return; /* current chunk still running, within timeout */
        }
        t->done += t->chunk;
        s_op_inflight = false;
    }

    for (;;)
    {
        if (t->done < t->size)
        {
            s_op_inflight = true; /* set before issuing so an immediate completion is accepted */
            flash_issue_chunk(t);
            return;
        }

        /* Task finished: release its data segment (if any) and pop it. */
        flash_task_abandon(t);
        if (s_q_count == 0u)
        {
            return;
        }
        t = &s_flash_queue[s_q_head];
    }
}

bool Flash_Async_IsIdle(void)
{
    return (s_q_count == 0u);
}

/*
 * Blocking wait: pumps the state machine until every queued task has finished.
 * Because new tasks are always appended at the tail of the queue, a task waited
 * on here never cuts ahead of an asynchronous operation that is already running
 * or queued before it -- it simply waits its turn. Must be called from a context
 * where the SROM interrupt can fire (task / main-loop context).
 */
void Flash_Async_Wait(void)
{
    while (s_q_count != 0u)
    {
        Flash_Async_Process();
    }
}

/* ------------------------------------------------------------------ */
/* SROM interrupt handler                                              */
/* ------------------------------------------------------------------ */
static void Flash_Isr(void)
{
    un_srom_api_resps_t apiResp;
    cy_en_srom_api_status_t sromDrvStatus = Cy_Srom_GetApiResponse(&apiResp);
    /* Only a response for an operation that is actually in flight is accepted;
       a late response from a timed-out chunk must not complete a later one. */
    if ((sromDrvStatus == CY_SROM_STATUS_SUCCESS) && s_op_inflight)
    {
        s_completeflag = true;
    }
    Cy_Flashc_InvalidateFlashCacheBuffer();
}

