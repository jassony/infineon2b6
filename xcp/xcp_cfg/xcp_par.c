/* -----------------------------------------------------------------------------
  Filename:    xcp_par.c
  Description: Toolversion: 02.03.11.01.70.09.79.00.00.00
               
               Serial Number: CBD1700979
               Customer Info: Huizhou Desay SV Automotive Co., Ltd.
                              Package: CBD_Vector_SLP2
                              Micro: R7F7016213AFP
                              Compiler: Green Hills 2015.1.7
               
               
               Generator Fwk   : GENy 
               Generator Module: Xcp
               
               Configuration   : D:\HVACProject\FORD\CAF_C490_BlackBox\SVN\02_Source Code\01_Development Workspace\01_Project\01_Application\02_Sources\01_CC\02_COM\Tool\GENy_CAN\CAF_C490_BlackBox_Drv.gny
               
               ECU: 
                       TargetSystem: Hw_Rh850Cpu
                       Compiler:     GreenHills
                       Derivates:    F1K
               
               Channel "Channel0":
                       Databasefile: D:\HVACProject\FORD\CAF_C490_BlackBox\SVN\02_Source Code\01_Development Workspace\01_Project\01_Application\02_Sources\01_CC\02_COM\Tool\GENy_CAN\MS1_CAN_C490_GAS_D_MY18_VP0_v1.dbc
                       Bussystem:    CAN
                       Manufacturer: Ford
                       Node:         HVAC_RCCM
               Channel "Channel1":
                       Databasefile: D:\HVACProject\FORD\CAF_C490_BlackBox\SVN\02_Source Code\01_Development Workspace\01_Project\01_Application\02_Sources\01_CC\02_COM\Tool\GENy_CAN\Demo_Ch1_NmJunior_Test.dbc
                       Bussystem:    CAN
                       Manufacturer: Ford
                       Node:         DemoEcu

 ----------------------------------------------------------------------------- */
/* -----------------------------------------------------------------------------
  C O P Y R I G H T
 -------------------------------------------------------------------------------
  Copyright (c) 2001-2015 by Vector Informatik GmbH. All rights reserved.
 
  This software is copyright protected and proprietary to Vector Informatik 
  GmbH.
  
  Vector Informatik GmbH grants to you only those rights as set out in the 
  license conditions.
  
  All other rights remain with Vector Informatik GmbH.
 -------------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */

#if !defined(__XCP_PAR_C__)
#define __XCP_PAR_C__

#include "XcpProf.h"

/* Events */
V_MEMROM0 V_MEMROM1 XcpCharType V_MEMROM2 kXcpEventName_0[] = "Event_1ms";
V_MEMROM0 V_MEMROM1 XcpCharType V_MEMROM2 kXcpEventName_1[] = "Event_2ms";
V_MEMROM0 V_MEMROM1 XcpCharType V_MEMROM2 kXcpEventName_2[] = "Event_10ms";
V_MEMROM0 V_MEMROM1 XcpCharType V_MEMROM2 kXcpEventName_3[] = "Event_100ms";
V_MEMROM0 V_MEMROM1 XcpCharType V_MEMROM2 V_MEMROM3* V_MEMROM1 V_MEMROM2 kXcpEventName[] = 
{
  &kXcpEventName_0[0], 
  &kXcpEventName_1[0], 
  &kXcpEventName_2[0],
  &kXcpEventName_3[0]
};
V_MEMROM0 V_MEMROM1 vuint8 V_MEMROM2 kXcpEventNameLength[] = 
{
  (vuint8) 9,
  (vuint8) 9,
  (vuint8) 10,
  (vuint8) 11
};
V_MEMROM0 V_MEMROM1 vuint8 V_MEMROM2 kXcpEventCycle[] = 
{
  (vuint8) 1,
  (vuint8) 2,
  (vuint8) 1,
  (vuint8) 1
};
V_MEMROM0 V_MEMROM1 vuint8 V_MEMROM2 kXcpEventUnit[] = 
{
  (vuint8) DAQ_TIMESTAMP_UNIT_1MS,
  (vuint8) DAQ_TIMESTAMP_UNIT_1MS,
  (vuint8) DAQ_TIMESTAMP_UNIT_10MS,
  (vuint8) DAQ_TIMESTAMP_UNIT_100MS
};
V_MEMROM0 V_MEMROM1 vuint8 V_MEMROM2 kXcpEventDirection[] = 
{
  (vuint8) DAQ_EVENT_DIRECTION_DAQ,
  (vuint8) DAQ_EVENT_DIRECTION_DAQ,
  (vuint8) DAQ_EVENT_DIRECTION_DAQ,
  (vuint8) DAQ_EVENT_DIRECTION_DAQ
};
/* Online calibration */
/* Flash programming */

/* begin Fileversion check */
#ifndef SKIP_MAGIC_NUMBER
#ifdef MAGIC_NUMBER
  #if MAGIC_NUMBER != 338824065
      #error "The magic number of the generated file <D:\HVACProject\FORD\CAF_C490_BlackBox\SVN\02_Source Code\01_Development Workspace\01_Project\01_Application\02_Sources\01_CC\02_COM\Tool\GENy_CAN\GenCode(Drv)\xcp_par.c> is different. Please check time and date of generated files!"
  #endif
#else
  #define MAGIC_NUMBER 338824065
#endif  /* MAGIC_NUMBER */
#endif  /* SKIP_MAGIC_NUMBER */

/* end Fileversion check */

#endif /* __XCP_PAR_C__ */
