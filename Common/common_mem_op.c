/****************************************************************************************************
* Copyright (c) xxxx xxxx Co.Ltd. All rights reserved.
*
* File Name     : common_mem_op.c
* Author        : yangming
* Date          : 2024-01-22
* Version       : 1.00
* Description   : Common memory operation file.
* Others        : None
*
****************************************************************************************************/
#include "common_mem_op.h"

/****************************************************************************************************
* Function Name : common_memcpy
* Description   : memory copy.
* Argument      : dest  :destination data pointer.
*                 src   :source data pointer.
*                 length:copy length.
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void common_memcpy(uint08* dest, uint08* src, uint32 length)
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

/****************************************************************************************************
* Function Name : common_memset
* Description   : memory set.
* Argument      : dest  :destination data pointer.
*                 val   :set data.
*                 length:copy length.
* Return Value  : void
* Notes         : None
****************************************************************************************************/
void common_memset(uint08* dest, uint32 val, uint32 length)
{
    if ((DEF_NULL == dest) || (0U == length))
    {
        return;
    }

    while (length--)
    {
        *dest = val;
        dest++;
    }
}

/****************************************************************************************************
* Function Name : common_compare
* Description   : memory compare.
* Argument      : buf1  :.
*                 buf2  :.
*                 length:copy length.
* Return Value  : void
* Notes         : None
****************************************************************************************************/
typ_bool common_compare(uint08* buf1, uint08* buf2, uint32 length)
{
    if ((DEF_NULL == buf1) || (DEF_NULL == buf2) || (0U == length))
    {
        return DEF_FALSE;
    }

    while (length--)
    {
        if (*buf1 != *buf2)
        {
            return DEF_FALSE;
        }
        else { /* continue */ }
        buf1++;
        buf2++;
    }

    return DEF_TRUE;
}

