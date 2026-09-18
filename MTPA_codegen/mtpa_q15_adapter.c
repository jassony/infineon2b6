#include "mtpa_q15_adapter.h"

#include "../ConfigWizard/Ifx_MDA_FluxEstimatorF16_Cfg.h"
#include "../ConfigWizard/Ifx_MS_FocSolutionF16_Cfg.h"
#include "mtpa_project_parameters.h"
#include "mtpa_reference_wrapper_ert_rtw/mtpa_reference_wrapper.h"
#include "pmsm_torque_estimator_wrapper_ert_rtw/pmsm_torque_estimator_wrapper.h"

void MTPA_Q15_initialize(void)
{
    pmsm_torque_estimator_wrapper_initialize();
    mtpa_reference_wrapper_initialize();
}

uint8_t MTPA_Q15_setMotorParameters(uint16_t directInductance_uH,
                                    uint16_t quadratureInductance_uH,
                                    uint8_t polePairs,
                                    uint16_t permanentMagnetFlux_mWb)
{
    return (uint8_t)MTPA_ProjectParameters_set(
        (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_CURRENT_A,
        (float)IFX_MDA_FLUXESTIMATORF16_CFG_BASE_ELEC_SPEED_RADPS,
        (float)IFX_MDA_FLUXESTIMATORF16_CFG_BASE_TORQUE_NM,
        (float)IFX_MS_FOCSOLUTIONF16_CFG_BASE_POWER_W,
        (float)permanentMagnetFlux_mWb * 0.001F,
        (float)directInductance_uH * 0.000001F,
        (float)quadratureInductance_uH * 0.000001F,
        (uint8_t)polePairs);
}

void MTPA_Q15_estimateTorque(const Ifx_Math_CmpFract16 *currentDq,
                             Ifx_Math_Fract16 speedQ15,
                             MTPA_Q15_TorqueEstimate *estimate)
{
    pmsm_torque_estimator_wrapper_U.Id_q15 = (int16_T)currentDq->real;
    pmsm_torque_estimator_wrapper_U.Iq_q15 = (int16_T)currentDq->imag;
    pmsm_torque_estimator_wrapper_U.Speed_q15 = (int16_T)speedQ15;
    pmsm_torque_estimator_wrapper_step();

    estimate->torqueQ15 = (Ifx_Math_Fract16)pmsm_torque_estimator_wrapper_Y.Te_q15;
    estimate->powerQ15 = (Ifx_Math_Fract16)pmsm_torque_estimator_wrapper_Y.Pe_q15;
}

void MTPA_Q15_computeReference(Ifx_Math_Fract16 torqueQ15,
                               Ifx_Math_Fract16 speedQ15,
                               Ifx_Math_CmpFract16 *referenceDq)
{
    mtpa_reference_wrapper_U.Tref_q15 = (int16_T)torqueQ15;
    mtpa_reference_wrapper_U.Speed_q15 = (int16_T)speedQ15;
    mtpa_reference_wrapper_step();

    referenceDq->real = (Ifx_Math_Fract16)mtpa_reference_wrapper_Y.Id_ref_q15;
    referenceDq->imag = (Ifx_Math_Fract16)mtpa_reference_wrapper_Y.Iq_ref_q15;
}
