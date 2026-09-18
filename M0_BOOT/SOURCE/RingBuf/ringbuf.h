/*******************************************************************************************************
* Copyright (c) Hangzhou Lingdong Automotive Thermal Management Technology Co.,Ltd. All rights reserved.
*
* File Name     : ringbuf.h
* Author        : yangming
* Date          : 2024-03-25
* Version       : 1.00
* Description   : ring buffer.
* Others        : None
*
*******************************************************************************************************/
#ifndef _RING_BUF_H
#define _RING_BUF_H
#include "platform_common_typdef.h"
typedef struct _ring_buf                        stRING_BUF;

struct _ring_buf
{
    uint08*                 buf;
    uint32                  in;
    uint32                  out;
    uint32                  size;
};

extern void ringbuf_init(stRING_BUF* rb, uint08* buffer, uint32 buf_size);
extern void ringbuf_put(stRING_BUF* rb, uint08* data, uint32 expect_len, uint32* real_len);
extern void ringbuf_get(stRING_BUF* rb, uint08* data, uint32 expect_len, uint32* real_len);
extern uint32 ringbuf_remain_len_get(stRING_BUF* rb);
extern uint32 ringbuf_owned_len_get(stRING_BUF* rb);

#endif /* _RING_BUF_H */
