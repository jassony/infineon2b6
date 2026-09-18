/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : ringbuf.c
* Author        : yangming
* Date          : 2024-03-25
* Version       : 1.00
* Description   : ring buffer.
* Others        : None
*
*******************************************************************************************************/
#include "ringbuf.h"
#include "common_mem_op.h"

#define RINGBUF_MIN(a, b)                       (((a) > (b)) ? (b) : (a))

void ringbuf_init(stRING_BUF* rb, uint08* buffer, uint32 buf_size)
{
    rb->buf     = buffer;
    rb->in      = 0U;
    rb->out     = 0U;
    rb->size    = buf_size;
}

void ringbuf_put(stRING_BUF* rb, uint08* data, uint32 expect_len, uint32* real_len)
{
    uint32 len = 0;
    uint32 free_len = RINGBUF_MIN(expect_len, (rb->size + rb->out - rb->in));

    if ((DEF_NULL == rb) || (DEF_NULL == data) || (0U == free_len))
    {
        return;
    }

    len = RINGBUF_MIN(free_len, (rb->size - (rb->in & (rb->size - 1))));

    common_memcpy(rb->buf + (rb->in & (rb->size - 1)), data, len);
    common_memcpy(rb->buf, data + len, free_len - len);

    rb->in += free_len;
    if (DEF_NULL != real_len)
    {
        *real_len = free_len;
    }
    else { /* none */ }
}

void ringbuf_get(stRING_BUF* rb, uint08* data, uint32 expect_len, uint32* real_len)
{
    uint32 len = 0;
    uint32 free_len = RINGBUF_MIN(expect_len, (rb->in - rb->out));

    if ((DEF_NULL == rb) || (DEF_NULL == data) || (0U == free_len))
    {
        return;
    }

    len = RINGBUF_MIN(free_len, (rb->size - (rb->out & (rb->size - 1))));

    common_memcpy(data, rb->buf + (rb->out & (rb->size - 1)), len);
    common_memcpy(data + len, rb->buf, free_len - len);

    rb->out += free_len;
    if (DEF_NULL != real_len)
    {
        *real_len = free_len;
    }
    else { /* none */ }
}

uint32 ringbuf_remain_len_get(stRING_BUF* rb)
{
    return (rb->size + rb->out - rb->in);
}

uint32 ringbuf_owned_len_get(stRING_BUF* rb)
{
    return (rb->in - rb->out);
}

