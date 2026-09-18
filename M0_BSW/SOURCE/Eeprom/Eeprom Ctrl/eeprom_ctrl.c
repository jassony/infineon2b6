/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : eeprom_ctrl.c
* Author        : yangming
* Date          : 2024-03-25
* Version       : 1.00
* Description   : EEPROM control.
* Others        : None
*
*******************************************************************************************************/
#include "eeprom_ctrl.h"
#include "eeprom_driver.h"
#include "eeprom_app_cfg.h"
#include "common_mem_op.h"
#include "eeprom_app_cbk.h"
#include "performance_test.h"

#define EEPROM_CTRL_FEED_DOG()  

static stRING_BUF s_eeprom_rb;
static uint08 s_eeprom_wr_buf[EE_WR_BUF_NUM];
static eEE_CTRL_STS s_eeprom_ctrl_sts;
static stEE_WR_INFO s_ee_wr_info;
static uint08 s_ee_wr_temp_buf[EE_WR_ITEM_BUF_SIZE]; /* Temporary buffer for the write item */
static uint08 s_ee_wr_temp1_buf[EE_WR_ITEM_BUF_SIZE]; /* Temporary buffer for the write item */
static stEE_ITEM_INFO s_cur_item_info;

static void eeprom_ctrl_write_process(void);
static void eeprom_item_write_idle(stEE_ITEM_INFO wr_item_info);
static void eeprom_item_write_start(stEE_ITEM_INFO wr_item_info);
static void eeprom_item_write_proceed(stEE_ITEM_INFO wr_item_info);
static void eeprom_item_write_chk(stEE_ITEM_INFO wr_item_info);


/****************************************************************************************************
* Function Name : eeprom_ctrl_init
* Description   : Eeprom control initialization function.
* Argument      : void
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void eeprom_ctrl_init(void)
{
    ringbuf_init((stRING_BUF*)&s_eeprom_rb, (uint08*)s_eeprom_wr_buf, EE_WR_BUF_NUM);
    s_eeprom_ctrl_sts = EE_WR_STS_IDLE;
    s_ee_wr_info.wr_sts = EE_WR_STS_IDLE;
    s_ee_wr_info.wr_cnt = EE_WR_CNT_NUM;
    s_ee_wr_info.wr_err = EE_CTRL_ERR_NONE;
    s_ee_wr_info.quotient = 0;
    s_ee_wr_info.remainder = 0;
    s_ee_wr_info.page = 0;
    soft_timer_set(&s_ee_wr_info.timer, EE_WR_DRV_FB_WAIT_TIME);
    soft_timer_set(&s_ee_wr_info.fast_timer, EE_WR_DRV_FB_WAIT_TIME);
}

/****************************************************************************************************
* Function Name : eeprom_ctrl_deinit
* Description   : Eeprom control de-initialization function.
* Argument      : void
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void eeprom_ctrl_deinit(void)
{
    eeprom_ctrl_init();
}

/****************************************************************************************************
* Function Name : eeprom_ctrl_read
* Description   : Eeprom read function.
* Argument      : item_info : This is a pointer to the infomation of read item
                  data : The data buffer
                  err : The error status of this item
* Return Value  : void
* Notes         : If err != EE_CTRL_RD_SUCCESS, read data fails.
****************************************************************************************************/
void eeprom_ctrl_read(stEE_ITEM_INFO item_info, uint08* data, eEE_CTRL_ERR* err)
{
    if (EE_CTRL_WR_PROCEED == s_eeprom_ctrl_sts)
    {
        *err = EE_CTRL_ERR_BUSY;
    }
    else
    {
        s_eeprom_ctrl_sts = EE_CTRL_RD_PROCEED;
        eeprom_drv_read(item_info.addr, data, item_info.len);
        s_eeprom_ctrl_sts = EE_CTRL_IDLE;
        if (DEF_TRUE == eeprom_drv_rd_sts_get())
        {
            *err = EE_CTRL_ERR_RD_SUCCESS;
        }
        else
        {
            *err = EE_CTRL_ERR_HW;
        }
    }
}

/****************************************************************************************************
* Function Name : eeprom_ctrl_fast_write
* Description   : Eeprom read function.
* Argument      : item_info : This is a pointer to the infomation of read item
                  data : The data buffer
* Return Value  : void
* Notes         : This function may need to be executed twice in a short period(e.g. 10ms).
****************************************************************************************************/
void eeprom_ctrl_fast_write(stEE_ITEM_INFO item_info, uint08* data)
{
    uint32 quotient = (item_info.len / EE_WR_PAGE_BYTE_SIZE);
    uint32 remainder = (item_info.len % EE_WR_PAGE_BYTE_SIZE);
    uint32 i = 0;

    if (s_eeprom_ctrl_sts != EE_CTRL_IDLE)
    {
        s_eeprom_ctrl_sts = EE_CTRL_IDLE;
    }
    else
    {
        eeprom_drv_write(item_info.addr + i * EE_WR_PAGE_BYTE_SIZE,
                         (uint08*)(data + i * EE_WR_PAGE_BYTE_SIZE),
                         EE_WR_PAGE_BYTE_SIZE);
        soft_timer_set(&s_ee_wr_info.fast_timer, EE_WR_DRV_FB_WAIT_TIME);

        for (i = 0; i < quotient; i++)
        {
            if (DEF_TRUE == is_soft_timer_timeout((stSOFT_TIMER*)&s_ee_wr_info.fast_timer))
            {
                eeprom_drv_write(item_info.addr + i * EE_WR_PAGE_BYTE_SIZE,
                                 (uint08*)(data + i * EE_WR_PAGE_BYTE_SIZE),
                                 EE_WR_PAGE_BYTE_SIZE);
                soft_timer_set(&s_ee_wr_info.fast_timer, EE_WR_DRV_FB_WAIT_TIME);
                EEPROM_CTRL_FEED_DOG();
            }
            else { /* wait */ }
        }

        if (remainder != 0)
        {
            eeprom_drv_write(item_info.addr + i * EE_WR_PAGE_BYTE_SIZE,
                             (uint08*)(data + i * EE_WR_PAGE_BYTE_SIZE),
                             remainder);
        }
        else { /* continue */ }
    }
}

/****************************************************************************************************
* Function Name : eeprom_ctrl_write_buf
* Description   : Write a certain length of data to the eeprom ring buffer;
                  This function is used to start the eeprom write operation.
* Argument      : item_info : This is a pointer to the infomation of read item
                  data : The data buffer
                  err : The error status of this item
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void eeprom_ctrl_write_buf(stEE_ITEM_INFO item_info, uint08* data, eEE_CTRL_ERR* err)
{
    uint32 temp = 0;
//    uint08 temp_buf[EE_WR_ITEM_TOTAL_SIZE];

    if (DEF_NULL == err)
    {
        return;
    }

    if (   (EE_CTRL_ERR_WR_PROCESS == *err)
        || (EE_CTRL_ERR_BUSY == *err)
        || (EE_CTRL_ERR_OTR == *err)
        )
    {
        return;
    }

    if (ringbuf_remain_len_get(&s_eeprom_rb) < item_info.len)
    {
        *err = EE_CTRL_ERR_BUF_OVF;
        return;
    }

    *err = EE_CTRL_ERR_WR_PROCESS;
    temp = EE_ITEM_OCCUPIED_BYTE;
//    /* Copy item information and data together */
//    common_memcpy((uint08*)temp_buf, (uint08*)&item_info, temp);
//    common_memcpy((uint08*)(temp_buf + temp), data, item_info.len);
    /* Data is writtten to the buffer, after which the temporary buffer is released */
    ringbuf_put(&s_eeprom_rb, (uint08*)&item_info, temp, DEF_NULL);
    ringbuf_put(&s_eeprom_rb, (uint08*)data, item_info.len, DEF_NULL);

    s_ee_wr_info.wr_cnt = EE_WR_CNT_NUM;
}

/****************************************************************************************************
* Function Name : eeprom_ctrl_process
* Description   : Eeprom control procssing function.
* Argument      : void
* Return Value  : void
* Notes         : This function need to be executed in a loop, which a recommended time of 10ms.
****************************************************************************************************/
void eeprom_ctrl_process(void)
{
    eeprom_ctrl_write_process();
}

/****************************************************************************************************
* Function Name : eeprom_ctrl_write_process
* Description   : Eeprom control write function.
* Argument      : void
* Return Value  : void
* Notes         : This function need to be executed in a loop, which a recommended time of 10ms.
****************************************************************************************************/
void eeprom_ctrl_write_process(void)
{
    if (EE_CTRL_RD_PROCEED == s_eeprom_ctrl_sts)
    {
        return;
    }
    else if (EE_CTRL_IDLE == s_eeprom_ctrl_sts)
    {
        if (ringbuf_owned_len_get(&s_eeprom_rb) <= EE_ITEM_OCCUPIED_BYTE)
        {
            return;
        }

        /* Obtain the write item information and release the corresponding ring buffer */
        ringbuf_get(&s_eeprom_rb, (uint08*)s_ee_wr_temp_buf, EE_ITEM_OCCUPIED_BYTE, DEF_NULL);
        common_memcpy((uint08*)&s_cur_item_info, (uint08*)s_ee_wr_temp_buf, EE_ITEM_OCCUPIED_BYTE);
        s_eeprom_ctrl_sts = EE_CTRL_WR_PROCEED;
    }
    else if (EE_CTRL_WR_PROCEED == s_eeprom_ctrl_sts)
    {
        switch (s_ee_wr_info.wr_sts)
        {
            case EE_WR_STS_IDLE:
                eeprom_item_write_idle(s_cur_item_info);
                break;
            case EE_WR_STS_START:
                eeprom_item_write_start(s_cur_item_info);
                break;
            case EE_WR_STS_PROCEED:
                eeprom_item_write_proceed(s_cur_item_info);
                break;
            case EE_WR_STS_CHK:
                eeprom_item_write_chk(s_cur_item_info);
                break;
            default:
                break;
        }
    }
    else {}
}

static void eeprom_item_write_idle(stEE_ITEM_INFO wr_item_info)
{
    ringbuf_get(&s_eeprom_rb, (uint08*)s_ee_wr_temp_buf, wr_item_info.len, DEF_NULL);
    s_ee_wr_info.wr_sts = EE_WR_STS_START;
    s_ee_wr_info.quotient = (wr_item_info.len / EE_WR_PAGE_BYTE_SIZE);
    s_ee_wr_info.remainder = (wr_item_info.len % EE_WR_PAGE_BYTE_SIZE);
    s_ee_wr_info.page = 0;
}

static void eeprom_item_write_start(stEE_ITEM_INFO wr_item_info)
{
    uint32 page = s_ee_wr_info.page;

    if (0 == s_ee_wr_info.wr_cnt)
    {
        s_eeprom_ctrl_sts = EE_CTRL_IDLE;
        s_ee_wr_info.wr_sts = EE_WR_STS_IDLE;
        return;
    }
    
    if (0 == page)
    {
        eeprom_drv_write(wr_item_info.addr + page * EE_WR_PAGE_BYTE_SIZE,
                         (uint08*)(s_ee_wr_temp_buf + page * EE_WR_PAGE_BYTE_SIZE),
                         EE_WR_PAGE_BYTE_SIZE);
        soft_timer_set(&s_ee_wr_info.timer, EE_WR_DRV_FB_WAIT_TIME);
        page++;
    }
    else
    {}

    if (DEF_TRUE == is_soft_timer_timeout(&s_ee_wr_info.timer))
    {
        if (page < s_ee_wr_info.quotient)
        {
            eeprom_drv_write(wr_item_info.addr + page * EE_WR_PAGE_BYTE_SIZE,
                             (uint08*)(s_ee_wr_temp_buf + page * EE_WR_PAGE_BYTE_SIZE),
                             EE_WR_PAGE_BYTE_SIZE);
            page++;
            if (0 == (s_ee_wr_info.remainder))
            {
                s_ee_wr_info.wr_sts = EE_WR_STS_PROCEED;
            }
            else {}
        }
        else
        {
            eeprom_drv_write(wr_item_info.addr + page * EE_WR_PAGE_BYTE_SIZE,
                             (uint08*)(s_ee_wr_temp_buf + page * EE_WR_PAGE_BYTE_SIZE),
                             s_ee_wr_info.remainder);
            page++;
            s_ee_wr_info.wr_sts = EE_WR_STS_PROCEED;
        }

        soft_timer_set(&s_ee_wr_info.timer, EE_WR_DRV_FB_WAIT_TIME);
    }
    else
    {
    }

    s_ee_wr_info.page = page;
}

static void eeprom_item_write_proceed(stEE_ITEM_INFO wr_item_info)
{
    (void)wr_item_info;

    if (DEF_TRUE == eeprom_drv_wr_sts_get())
    {
        s_ee_wr_info.wr_sts = EE_WR_STS_CHK;
        s_ee_wr_info.wr_err = EE_CTRL_ERR_NONE;
        eeprom_app_copy_wr_err((eEE_WR_ITEM_TYP)s_cur_item_info.item, &s_ee_wr_info.wr_err);
    }
    else
    {
        if (DEF_TRUE == is_soft_timer_timeout(&s_ee_wr_info.timer))
        {
            s_ee_wr_info.wr_sts = EE_WR_STS_START;
            s_ee_wr_info.wr_cnt--;
            s_ee_wr_info.wr_err = EE_CTRL_ERR_HW;
            eeprom_app_copy_wr_err((eEE_WR_ITEM_TYP)s_cur_item_info.item, &s_ee_wr_info.wr_err);
        }
        else
        {
            /* wait */
        }
    }
}

static void eeprom_item_write_chk(stEE_ITEM_INFO wr_item_info)
{
//    uint08 s_ee_wr_temp1_buf[EE_WR_ITEM_TOTAL_SIZE];

    common_memset((uint08*)s_ee_wr_temp1_buf, EE_WR_ITEM_TOTAL_SIZE, 0U);
    eeprom_drv_read(wr_item_info.addr, (uint08*)s_ee_wr_temp1_buf, wr_item_info.len);
    if (DEF_TRUE == common_compare((uint08*)s_ee_wr_temp1_buf, (uint08*)s_ee_wr_temp_buf, wr_item_info.len))
    {
        s_eeprom_ctrl_sts = EE_CTRL_IDLE;
        s_ee_wr_info.wr_sts = EE_WR_STS_IDLE;
        s_ee_wr_info.wr_err = EE_CTRL_ERR_WR_SUCCESS;
        eeprom_app_copy_wr_err((eEE_WR_ITEM_TYP)s_cur_item_info.item, &s_ee_wr_info.wr_err);
    }
    else
    {
        s_ee_wr_info.wr_sts = EE_WR_STS_START;
        s_ee_wr_info.wr_cnt--;
        s_ee_wr_info.wr_err = EE_CTRL_ERR_OTR;
        eeprom_app_copy_wr_err((eEE_WR_ITEM_TYP)s_cur_item_info.item, &s_ee_wr_info.wr_err);
    }
}

