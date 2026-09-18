/***************************************************************************//**
* \file main_cm0plus.c
*
* \brief
* Main file for CM0+
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "m0_task.h"

int main(void)
{
    task_init();

    while (true)
    {
        /* Wait 0.05 [s] */
    task_process();
    }
}


/* [] END OF FILE */
