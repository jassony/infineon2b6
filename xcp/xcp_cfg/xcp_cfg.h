/* -----------------------------------------------------------------------------
  Filename:    xcp_cfg.h
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

#if !defined(__XCP_CFG_H__)
#define __XCP_CFG_H__

#define XCP_CANMSG_RXID   0x18FF0010u
#define XCP_CANMSG_TXID   0x18FF0011u

/*
 * Writable static-data windows from linker_directives_tviibe512k.icf.
 * The regions stop before HEAP_STACK on each core. They therefore cover
 * global/static application variables, including variables added later, but
 * exclude SROM, heap, stack, flash and peripheral address ranges.
 */
#define XCP_M0_STATIC_RAM_ADDRESS (0x08000800u)
#define XCP_M0_STATIC_RAM_SIZE    (0x00007400u)
#define XCP_M4_STATIC_RAM_ADDRESS (0x08008000u)
#define XCP_M4_STATIC_RAM_SIZE    (0x00006000u)

#define DATA_CAL //__attribute__((section(".customSection")))

#define XCP_CPUTYPE_LITTLEENDIAN
//#define V_ENABLE_USE_DUMMY_STATEMENT

/* General settings */
#define XCP_ENABLE_PARAMETER_CHECK
#define XCP_ENABLE_CALIBRATION
#define XCP_DISABLE_SEND_EVENT
#define XCP_DISABLE_TESTMODE
#define XCP_DISABLE_BOOTLOADER_DOWNLOAD
#define XCP_ENABLE_WRITE_PROTECTION
#define XCP_DISABLE_READ_PROTECTION
#define XCP_DISABLE_CONTROL//XCP_ENABLE_CONTROL
#define XCP_DISABLE_GET_XCP_DATA_POINTER
#define XCP_DISABLE_OPENCMDIF
#define XCP_DISABLE_DEV_ERROR_DETECT
#define XCP_DISABLE_VECTOR_GENERICMEASUREMENT
#define XCP_DISABLE_AMD_RUNTIME_MEASUREMENT
#define XCP_ENABLE_GET_SESSION_STATUS_API//XCP_DISABLE_GET_SESSION_STATUS_API
#define XCP_DISABLE_ADDRESS_DOUBLING_SUPPORT
#define XCP_DISABLE_USE_BYTE_ACCESS
/*  */
/* EEPROM access */
#define XCP_DISABLE_READ_EEPROM
#define XCP_DISABLE_WRITE_EEPROM
/*  */
/* Service request message */
#define XCP_DISABLE_SERV_TEXT
#define XCP_DISABLE_SERV_TEXT_PUTCHAR
#define XCP_DISABLE_SERV_TEXT_PRINT
/*  */
/* Standard commands */
#define XCP_ENABLE_COMM_MODE_INFO
#define XCP_DISABLE_SEED_KEY
#define XCP_DISABLE_MODIFY_BITS
#define XCP_DISABLE_SHORT_DOWNLOAD
#define XCP_DISABLE_USER_COMMAND
#define XCP_DISABLE_GET_ID_GENERIC
#define XCP_DISABLE_TL_COMMAND
/*  */
/* Block transfer */
#define XCP_DISABLE_BLOCK_UPLOAD
#define XCP_DISABLE_BLOCK_DOWNLOAD
#define kXcpBlockDownloadMinSt               0u
/*  */
/* Checksum */
#define XCP_DISABLE_CHECKSUM
/*  */
/* Synchronous Data Acquisition (DAQ) */
#define XCP_ENABLE_DAQ
#define kXcpDaqMemSize                       0x1000u
#define XCP_DISABLE_STIM
#define XCP_DISABLE_UNALIGNED_MEM_ACCESS
#define XCP_ENABLE_SEND_QUEUE
#define XCP_DISABLE_DAQ_PRESCALER
//#define XCP_ENABLE_DAQ_OVERRUN_INDICATION
#define XCP_DISABLE_DAQ_OVERRUN_INDICATION
#define XCP_DISABLE_WRITE_DAQ_MULTIPLE
#define XCP_DISABLE_DAQ_HDR_ODT_DAQ
#define XCP_DISABLE_DAQ_RESUME
#define XCP_ENABLE_DAQ_PROCESSOR_INFO
#define XCP_ENABLE_DAQ_RESOLUTION_INFO
#define XCP_DISABLE_DAQ_RUNTIME_SUPERVISION
/*  */
/* Events */
#define XCP_ENABLE_DAQ_EVENT_INFO
#define kXcpMaxEvent                         4u
/*  */
/* DAQ Timestamp */
#define XCP_DISABLE_DAQ_TIMESTAMP
#define XCP_DISABLE_DAQ_TIMESTAMP_FIXED
/*  */
/* Page switching */
#define XCP_DISABLE_CALIBRATION_PAGE
/*  */
/* Programming */
#define XCP_DISABLE_PROGRAM
#define XCP_DISABLE_SILENT_CHECK
#define CP_XCPDLL_VERSION                    0x0220u
#define CP_XCPDLL_RELEASE_VERSION            0x09u
/* -----------------------------------------------------------------------------
    &&&~ Multiple transport layer support
 ----------------------------------------------------------------------------- */
/*The following change accroding to the NEVS XCP, mark by LT 20200118*/
/* Multiple transport layer support */
#define NUMBER_OF_TRANSPORTLAYERS            1u
/* CompilerAbstractionEnabled == FALSE */
/* 1 */
#define XCP_TRANSPORT_LAYER_CAN              0


/* Transport Layer */
/*  */
#define kXcpNumberOfCanChannels              1
#define XCP_DISABLE_CAN_MULTI_CHANNEL
#define XCP_DISABLE_CAN_MULTI_CONNECTION_PROTECTION
#define XCP_TRANSPORT_LAYER_TYPE_CAN
#define XCP_TRANSPORT_LAYER_VERSION_CAN      0x0100
#define kCanXcpMaxCTO                        8
#define kCanXcpMaxDTO                        8
#define XCP_DISABLE_CAN_VARIABLE_DLC
#define kXcpSendQueueMinSize                 1
/*  */
#define CP_XCPONCANDLL_VERSION               0x0108u
#define CP_XCPONCANDLL_RELEASE_VERSION       0x03u

/* begin Fileversion check */
#ifndef SKIP_MAGIC_NUMBER
#ifdef MAGIC_NUMBER
  #if MAGIC_NUMBER != 338824065
      #error "The magic number of the generated file <D:\HVACProject\FORD\CAF_C490_BlackBox\SVN\02_Source Code\01_Development Workspace\01_Project\01_Application\02_Sources\01_CC\02_COM\Tool\GENy_CAN\GenCode(Drv)\xcp_cfg.h> is different. Please check time and date of generated files!"
  #endif
#else
  #define MAGIC_NUMBER 338824065
#endif  /* MAGIC_NUMBER */
#endif  /* SKIP_MAGIC_NUMBER */

/* end Fileversion check */

//#define CAL_RAM  __attribute__((section(".cal_data_2")))

#endif /* __XCP_CFG_H__ */
