/*
 * rte.c
 *
 *  Created on: 2026Äê2ÔÂ24ÈÕ
 *      Author: hzldy
 */


#include "m0_boot_rte.h"

static void Rte_Bsw_To_Swc(void);
static void Rte_Swc_To_Bsw(void);

void Rte_Process(void)
{
    Rte_Bsw_To_Swc();
    Rte_Swc_To_Bsw();
}

static void Rte_Bsw_To_Swc(void)
{

  
}


static void Rte_Swc_To_Bsw(void)
{


}
