/*
 * m0_m4_ipc.c
 *
 *  Created on: 2026Äê2ÔÂ6ÈÕ
 *      Author: hzldy
 */
#include "m0_m4_ipc.h"


#ifdef USE_M4
#include "mcu_load_profiler.h"

ST_M0_GET_M4_SET_DATA m4_set_data ;
ST_M4_GET_M0_SET_DATA m4_get_data;

bool released;


cy_stc_ipc_pipe_config_t pipeConfig = 
{
    .epIndexForThisCpu = EP_INDEX_THIS_CPU,
    .epConfigData      = CY_IPC_PIPE_ENDPOINTS_DEFAULT_CONFIG,
};



void ReceivedCallback(uint32_t *msgPtr)
{
  ST_M4_GET_M0_SET_DATA * ptr = (ST_M4_GET_M0_SET_DATA *)msgPtr;
  if(ptr->checksum == IPC_Checksum_Count(&ptr->clientId,sizeof(ST_M4_GET_M0_SET_DATA)/sizeof(uint32_t) -1))
    memcpy(&m4_get_data,msgPtr,sizeof(ST_M4_GET_M0_SET_DATA));
};


void ReleaseCallback(void)
{
    /* Notified core already got the data. */
    /* Update send data for next transmission. */
    released = true;
}

#if (MCU_LOAD_PROFILER_ENABLE != 0u)
static void M4_IpcPipeIsr(void)
{
    McuLoadProfilerToken profilerToken = McuLoadProfiler_begin();

    Cy_IPC_Pipe_Isr();
    (void)McuLoadProfiler_end(MCU_LOAD_CONTEXT_IPC_IRQ, profilerToken);
}
#endif


void Ipc_Init_Pipe(void)
{
    Cy_IPC_Pipe_Init(&pipeConfig);
#if (MCU_LOAD_PROFILER_ENABLE != 0u)
    Cy_SysInt_SetSystemIrqVector(
        (cy_en_intr_t)CY_IPC_INTR_NUM_TO_VECT(
            pipeConfig.epConfigData[EP_INDEX_THIS_CPU].ipcIntrStructNr),
        M4_IpcPipeIsr);
#endif

    /* Register data received callback */
    Cy_IPC_Pipe_RegisterCallback
    (
        ReceivedCallback,
        0x01u  /* Accept Client ID = 0x01 */
    );

    /* Already registered the handler to the system interrup structure.
     * So just enable corresponding IRQ channel.
     */
//    NVIC_SetPriority(CPUIntIdx0_IRQn,1);
//    NVIC_SetPriority(CPUIntIdx1_IRQn,1);
//    NVIC_SetPriority(CPUIntIdx7_IRQn,3);

    NVIC_ClearPendingIRQ(pipeConfig.epConfigData[EP_INDEX_THIS_CPU].ipcCpuIntIdx);
    NVIC_EnableIRQ(pipeConfig.epConfigData[EP_INDEX_THIS_CPU].ipcCpuIntIdx);

}


void Ipc_Pipe_Set(void)
{
  cy_en_ipc_pipe_status_t status ;
  m4_set_data.checksum = IPC_Checksum_Count(&m4_set_data.clientId,sizeof(ST_M0_GET_M4_SET_DATA)/sizeof(uint32_t) -1);
  status= Cy_IPC_Pipe_SendMessage(EP_INDEX_OTHER_CPU,&m4_set_data,ReleaseCallback);

}

#elif defined(USE_M0)

ST_M0_GET_M4_SET_DATA m0_get_data ;
ST_M4_GET_M0_SET_DATA m0_set_data = 
{
  .clientId = 1,

};


cy_stc_ipc_pipe_config_t pipeConfig = 
{
    .epIndexForThisCpu = EP_INDEX_THIS_CPU,
    .epConfigData      = CY_IPC_PIPE_ENDPOINTS_DEFAULT_CONFIG,
};
bool released;


void ReleaseCallback(void)
{
    /* Notified core already got the data. */
    /* Update send data for next transmission. */
    released = true;
}

void ReceivedCallback(uint32_t *msgPtr)
{
  ST_M0_GET_M4_SET_DATA *ptr = (ST_M0_GET_M4_SET_DATA *)msgPtr;
  if(ptr->checksum == IPC_Checksum_Count(&ptr->clientId,sizeof(ST_M0_GET_M4_SET_DATA)/sizeof(uint32_t) -1))
    memcpy(&m0_get_data,msgPtr,sizeof(ST_M0_GET_M4_SET_DATA));
};


void Ipc_Init_Pipe(void)
{
    Cy_IPC_Pipe_Init(&pipeConfig);
    Cy_IPC_Pipe_RegisterCallback(ReceivedCallback,0x00);
    NVIC_ClearPendingIRQ(pipeConfig.epConfigData[EP_INDEX_THIS_CPU].ipcCpuIntIdx);
    NVIC_EnableIRQ(pipeConfig.epConfigData[EP_INDEX_THIS_CPU].ipcCpuIntIdx);

}

void Ipc_Pipe_Set(void)
{
  cy_en_ipc_pipe_status_t status ;
  m0_set_data.checksum = IPC_Checksum_Count(&m0_set_data.clientId,sizeof(ST_M4_GET_M0_SET_DATA)/sizeof(uint32_t) -1);
  status= Cy_IPC_Pipe_SendMessage(EP_INDEX_OTHER_CPU,&m0_set_data,ReleaseCallback);
  
}

#endif

uint32_t IPC_Checksum_Count(uint32_t *ptr,uint16_t len)
{
  uint32_t data = 0;
  for(uint16_t i = 0 ;i < len ; i++)
  {
    data += *ptr;
    ptr ++;
  
  }
  return ~data;
}
