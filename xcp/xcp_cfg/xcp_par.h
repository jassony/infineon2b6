/* -----------------------------------------------------------------------------
  Filename:    xcp_par.h
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

#if !defined(__XCP_PAR_H__)
#define __XCP_PAR_H__

#include <stdint.h>

#define V_MEMRAM0
#define V_MEMRAM1
#define V_MEMRAM2
#define V_MEMROM0
#define V_MEMROM1
#define V_MEMROM2
#define V_MEMROM3

#define kCanNoCopyData    0u
#define kCanCopyData      1u

#define kCanTxOk          0u
#define kCanTxNOk         1u

typedef unsigned char XcpCharType;
typedef uint8_t vuint8;
typedef uint16_t vuint16;
typedef uint32_t vuint32;

typedef struct
{
  vuint32 msgId;
  vuint8 data[8];
} XcpCanMessageType;

extern XcpCanMessageType Xcp_message_RXD;
extern XcpCanMessageType Xcp_message_TXD;

#define CanRxInfoStructPtr const XcpCanMessageType*
#define rxRegPtr (rxStruct->data)

#define C_SINGLE_RECEIVE_CHANNEL
#define C_SINGLE_RECEIVE_BUFFER

/* Events */
V_MEMROM0 extern  V_MEMROM1 XcpCharType V_MEMROM2 V_MEMROM3* V_MEMROM1 V_MEMROM2 kXcpEventName[];
V_MEMROM0 extern  V_MEMROM1 vuint8 V_MEMROM2 kXcpEventNameLength[];
V_MEMROM0 extern  V_MEMROM1 vuint8 V_MEMROM2 kXcpEventCycle[];
V_MEMROM0 extern  V_MEMROM1 vuint8 V_MEMROM2 kXcpEventUnit[];
V_MEMROM0 extern  V_MEMROM1 vuint8 V_MEMROM2 kXcpEventDirection[];
#define XcpEventChannel_1ms                  0u
#define XcpEventChannel_2ms                  1u
#define XcpEventChannel_10ms                 2u
#define XcpEventChannel_100ms                3u
/*  */
/* Online calibration */
/*  */
/* Flash programming */
/*  */
/* DAQ */
/*  */
/* Checksum */
/*  */
/*Add by LT 20200118-base on NEVS XCP BSW*/
/* Transport Layer */
extern vuint8 XcpCanIf_Transmit(void);
extern void XcpPort_EnterCritical(void);
extern void XcpPort_ExitCritical(void);

#define XcpGetCanTransmitDataPtr()           Xcp_message_TXD.data
#define ApplXcpInterruptDisable()            XcpPort_EnterCritical()
#define ApplXcpInterruptEnable()             XcpPort_ExitCritical()

#define XcpTransmit()                   XcpCanIf_Transmit()

/* begin Fileversion check */
#ifndef SKIP_MAGIC_NUMBER
#ifdef MAGIC_NUMBER
  #if MAGIC_NUMBER != 338824065
      #error "The magic number of the generated file <D:\HVACProject\FORD\CAF_C490_BlackBox\SVN\02_Source Code\01_Development Workspace\01_Project\01_Application\02_Sources\01_CC\02_COM\Tool\GENy_CAN\GenCode(Drv)\xcp_par.h> is different. Please check time and date of generated files!"
  #endif
#else
  #define MAGIC_NUMBER 338824065
#endif  /* MAGIC_NUMBER */
#endif  /* SKIP_MAGIC_NUMBER */

/* end Fileversion check */

#endif /* __XCP_PAR_H__ */
