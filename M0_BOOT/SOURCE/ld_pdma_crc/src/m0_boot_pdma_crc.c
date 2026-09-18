/*
 * m0_boot_pdma_crc.c
 *
 *  Created on: 2026Äê3ÔÂ20ÈÕ
 *      Author: hzldy
 */


#include "m0_boot_pdma_crc.h"

#define BUFFER_SIZE         5
#define DW_CHANNEL          0

static  volatile stc_DW_t*          pstcDW          =   DW0;
static  cy_stc_pdma_descr_t         stcDescr;
const   uint8_t                     au8SrcBuffer[]   =   {0x12,0x34,0x56,0x78,0x9a};

const   cy_stc_pdma_crc_config_t    stcCrcConfig    =   {
                                                        .data_reverse = 1,                  
                                                        .rem_reverse  = 1,              
                                                        .data_xor     = 0,                      
                                                        .polynomial   = 0x04c11db7,             
                                                        .lfsr32       = 0xFFFFFFFF,                     
                                                        .rem_xor      = 0xFFFFFFFF,                       
                                                        };
const   cy_stc_pdma_chnl_config_t   chnlConfig      =   {
                                                        .PDMA_Descriptor=   &stcDescr,
                                                        .preemptable    =   0,
                                                        .priority       =   0,
                                                        .enable         =   1,  /*enabled after initialization*/
                                                        };

static  cy_stc_pdma_descr_config_t  stcDmaDescrConfig=  {
                                                        .deact          =   0,  /*Do not wait for trigger de-activation*/
                                                        .intrType       =   CY_PDMA_INTR_1ELEMENT_CMPLT,
                                                        .trigoutType    =   CY_PDMA_TRIGOUT_1ELEMENT_CMPLT,
                                                        .chStateAtCmplt =   CY_PDMA_CH_DISABLED, 
                                                        .triginType     =   CY_PDMA_TRIGIN_DESCR,
                                                        .dataSize       =   CY_PDMA_BYTE,
                                                        .srcTxfrSize    =   0,  /*= dataSize*/
                                                        .destTxfrSize   =   0,  /*= dataSize*/
                                                        .descrType      =   CY_PDMA_CRC_TRANSFER,
                                                        .srcAddr        =   (void*) au8SrcBuffer,
                                                        .destAddr       =   0,          //below initialized
                                                        .srcXincr       =   1,
                                                        .destXincr      =   1,
                                                        .xCount         =   BUFFER_SIZE,
                                                        .srcYincr       =   0,
                                                        .destYincr      =   0,
                                                        .yCount         =   0,
                                                        };

static  uint32_t                    u32CrcResult   =    0;




uint32_t Pdma_Crc32_Cal(uint8_t* data,uint32_t size)
{
    Cy_PDMA_Disable(pstcDW);
    Cy_PDMA_CRC_Config( pstcDW,&stcCrcConfig);
    stcDmaDescrConfig.destAddr = (void *)&pstcDW->unCRC_LFSR_CTL.u32Register;     
    stcDmaDescrConfig.srcAddr = data;
    stcDmaDescrConfig.xCount = size;
    Cy_PDMA_Descr_Init(&stcDescr,&stcDmaDescrConfig);
    Cy_PDMA_Chnl_Init( pstcDW,DW_CHANNEL,(const cy_stc_pdma_chnl_config_t*) &chnlConfig);
    Cy_PDMA_Enable(pstcDW);
    Cy_TrigMux_SwTrigger(TRIG_OUT_MUX_0_PDMA0_TR_IN0,TRIGGER_TYPE_CPUSS_DW0_TR_IN__EDGE,1);   
    while(pstcDW->CH_STRUCT[DW_CHANNEL].unCH_CTL.stcField.u1ENABLED)
      ;
    return Cy_PDMA_GetCrcRemainderResult(pstcDW);
  

}













