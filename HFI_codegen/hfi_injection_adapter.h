#ifndef HFI_INJECTION_ADAPTER_H
#define HFI_INJECTION_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

#include "Ifx_Math.h"

extern volatile uint8_t Cal_Hfi_Enable_u8;
extern volatile float Cal_Hfi_Amplitude_V_f32;
extern volatile float Cal_Hfi_Frequency_Hz_f32;

void HfiInjection_init(void);
void HfiInjection_reset(void);
Ifx_Math_PolarFract16 HfiInjection_applyPolar(Ifx_Math_PolarFract16 voltageCommandPolar,
                                              bool kreFocAllowed);
/* Latched result of the most recent applyPolar() call. Diagnostic consumers
 * use it to reject the exact PWM sample that contained carrier injection. */
bool HfiInjection_isOutputActive(void);

#endif /* HFI_INJECTION_ADAPTER_H */
