#include "../fwc_reference_wrapper_ert_rtw/fwc_reference_wrapper.h"
#include "../fwc_q15_adapter.h"

#include <assert.h>
#include <stdio.h>

static float absoluteValue(const float value)
{
    return (value < 0.0F) ? -value : value;
}

static void setNominalInputs(void)
{
    uint8_T index;

    fwc_reference_wrapper_U.Enable = 1U;
    fwc_reference_wrapper_U.Reset = 0U;
    fwc_reference_wrapper_U.Eligible = 1U;
    fwc_reference_wrapper_U.VdcValid = 1U;
    fwc_reference_wrapper_U.VutilReq = 0.94F;
    fwc_reference_wrapper_U.VutilAct = 0.94F;
    fwc_reference_wrapper_U.IdBase = 0.0F;
    fwc_reference_wrapper_U.Params[0] = 0.95F;
    fwc_reference_wrapper_U.Params[1] = 0.5F;
    fwc_reference_wrapper_U.Params[2] = 40.0F;
    fwc_reference_wrapper_U.Params[3] = -13107.0F / 32768.0F;
    fwc_reference_wrapper_U.Params[4] = 4.0F;
    fwc_reference_wrapper_U.Params[5] = 1.0F;
    fwc_reference_wrapper_U.Params[6] = 0.01F;
    fwc_reference_wrapper_U.Params[7] = 5.0F;
    fwc_reference_wrapper_U.Params[8] = 0.02F;
    fwc_reference_wrapper_U.Params[9] = 2.0F;
    fwc_reference_wrapper_U.Params[10] = 20.0F;
    fwc_reference_wrapper_U.FloorSatTicks = 0U;
    fwc_reference_wrapper_U.UnsatTicks = 40U;
    for (index = 0U; index < 16U; ++index)
    {
        fwc_reference_wrapper_DW.State_DSTATE[index] = 0.0F;
    }
}

static void compareProductionAdapter(void)
{
    Fwc_Q15_VoltageSnapshot s = {0};
    Fwc_Q15_Output out;
    Fwc_Q15_FastConfig config = {0};
    uint16_t k;
    uint8_t eligible;
    int16_t baseId;
    int32_t referenceQ15;
    uint32_t entriesBefore = Meas_FWC_RecoveryCnt_u32;
    s.dcLinkVoltageQ15 = 30000;
    fwc_reference_wrapper_initialize();
    setNominalInputs();
    Fwc_Q15_reset();
    for (k = 0u; k < 1400u; ++k)
    {
        float req = 0.90F;
        float act = 0.90F;
        float reqPu;
        float actPu;
        if (((k >= 100u) && (k < 400u)) || (k == 440u) || (k >= 1130u))
        {
            req = 1.20F; act = 1.10F;
        }
        else if ((k >= 400u) && (k < 420u))
        {
            req = 1.00F; act = 0.994F;
        }
        s.requestedVoltageQ15 = (int16_t)(req * 30000.0F / 1.7320508F + 0.5F);
        s.actualVoltageQ15 = (int16_t)(act * 30000.0F / 1.7320508F + 0.5F);
        reqPu = 1.7320508F * (float)s.requestedVoltageQ15 / 30000.0F;
        actPu = 1.7320508F * (float)s.actualVoltageQ15 / 30000.0F;
        s.valid = ((k >= 1110u) && (k < 1120u)) ? 0u : 1u;
        eligible = ((k >= 1120u) && (k < 1130u)) ? 0u : 1u;
        s.saturated = ((reqPu - actPu) > 0.01F) ? 1u : 0u;
        s.saturationStreakFast = s.saturated ? s.saturationStreakFast + 10u : 0u;
        s.unsaturationStreakFast = s.saturated ? 0u : s.unsaturationStreakFast + 10u;
        s.idAtFloorSaturationStreakFast = (s.saturated && config.enabled && config.idAtFloor)
            ? s.idAtFloorSaturationStreakFast + 10u : 0u;
        s.sequence += 2u;
        baseId = (k >= 600u) ? -3000 : 0;
        Cal_FWC_Enable_u8 = ((k >= 1100u) && (k < 1110u)) ? 0u : 1u;
        Cal_FWC_LpfTau_ms_f32 = (k >= 500u) ? 0.0F : 5.0F;
        Cal_FWC_Hyst_PU_f32 = (k >= 1150u) ? 0.01F : 0.02F;
        fwc_reference_wrapper_U.Enable = Cal_FWC_Enable_u8;
        fwc_reference_wrapper_U.Eligible = eligible;
        fwc_reference_wrapper_U.VdcValid = s.valid;
        fwc_reference_wrapper_U.VutilReq = reqPu;
        fwc_reference_wrapper_U.VutilAct = actPu;
        fwc_reference_wrapper_U.IdBase = (float)baseId / 32768.0F;
        fwc_reference_wrapper_U.Params[7] = Cal_FWC_LpfTau_ms_f32;
        fwc_reference_wrapper_U.Params[8] = Cal_FWC_Hyst_PU_f32;
        fwc_reference_wrapper_U.FloorSatTicks = s.idAtFloorSaturationStreakFast;
        fwc_reference_wrapper_U.UnsatTicks = s.unsaturationStreakFast;
        fwc_reference_wrapper_step();
        Fwc_Q15_execute(&s, eligible, baseId, (k & 1u) ? -32768 : 32767, &out);
        assert(Fwc_Q15_getFastConfig(&config) != 0u);
        referenceQ15 = (int32_t)(fwc_reference_wrapper_Y.IdRef * 32768.0F - 0.5F);
        assert(absoluteValue((float)out.idReferenceQ15 - (float)referenceQ15) <= 1.0F);
        assert(out.active == fwc_reference_wrapper_Y.Active);
        assert(out.valid == fwc_reference_wrapper_Y.Valid);
        assert(out.recoveryActive == fwc_reference_wrapper_Y.RecoveryAct);
        assert(Meas_FWC_WeakAct_u8 == fwc_reference_wrapper_Y.WeakAct);
        assert(Meas_FWC_Stat_u8 == fwc_reference_wrapper_Y.Status);
        assert(absoluteValue(Meas_FWC_VreqFlt_PU_f32 - fwc_reference_wrapper_Y.ReqFlt) < 1.0e-6F);
        assert(absoluteValue(Meas_FWC_VactFlt_PU_f32 - fwc_reference_wrapper_Y.ActFlt) < 1.0e-6F);
    }
    assert(Meas_FWC_RecoveryCnt_u32 >= entriesBefore + 2u);
    puts("FWC production/ERT comparison passed: 1400 samples");
}

int main(void)
{
    uint16_T sample;
    static const real32_T goldenRequest[] = {
        0.99F, 0.99F, 0.99F, 0.99F, 0.90F, 0.90F, 1.20F, 1.20F
    };
    static const real32_T goldenActual[] = {
        0.99F, 0.99F, 0.99F, 0.99F, 0.90F, 0.90F, 1.10F, 1.10F
    };
    static const real32_T goldenIdReference[] = {
        0.0F, 0.0F, 0.0F, -0.002F, -0.004F, -0.0035F, -0.0055F, -0.0075F
    };
    static const real32_T goldenReqFlt[] = {
        0.99000001F,0.99000001F,0.99000001F,0.99000001F,
        0.981818199F,0.974380195F,0.994891107F,1.01353741F
    };
    static const real32_T goldenActFlt[] = {
        0.99000001F,0.99000001F,0.99000001F,0.99000001F,
        0.981818199F,0.974380195F,0.985800207F,0.996182024F
    };
    static const uint8_T goldenStatus[] = { 1U, 1U, 1U, 1U, 1U, 1U, 2U, 2U };

    fwc_reference_wrapper_initialize();
    setNominalInputs();

    /* Values are the MATLAB single-precision golden vector in
     * run_fwc_reference_replay.m. Compare every ERT output sample, not
     * merely its final saturation result. */
    for (sample = 0U; sample < 8U; ++sample)
    {
        fwc_reference_wrapper_U.VutilReq = goldenRequest[sample];
        fwc_reference_wrapper_U.VutilAct = goldenActual[sample];
        fwc_reference_wrapper_step();
        assert(fwc_reference_wrapper_Y.Active == 1U);
        assert(fwc_reference_wrapper_Y.Valid == 1U);
        assert(absoluteValue(fwc_reference_wrapper_Y.IdRef
            - goldenIdReference[sample]) < 1.0e-6F);
        assert(fwc_reference_wrapper_Y.Status == goldenStatus[sample]);
        assert(absoluteValue(fwc_reference_wrapper_Y.ReqFlt - goldenReqFlt[sample]) < 1.0e-6F);
        assert(absoluteValue(fwc_reference_wrapper_Y.ActFlt - goldenActFlt[sample]) < 1.0e-6F);
        assert(fwc_reference_wrapper_Y.WeakAct == ((sample < 3U) ? 0U : 1U));
    }

    fwc_reference_wrapper_U.VutilReq = 1.20F;
    fwc_reference_wrapper_U.VutilAct = 1.10F;
    for (sample = 0U; sample < 260U; ++sample)
    {
        fwc_reference_wrapper_step();
    }
    assert(fwc_reference_wrapper_Y.IdRef >= -0.40001F);
    assert(fwc_reference_wrapper_Y.IdRef <= -0.39999F);
    assert(fwc_reference_wrapper_Y.Saturated == 1U);
    assert(fwc_reference_wrapper_Y.IdAtLo == 1U);
    assert(fwc_reference_wrapper_Y.Status == 2U);

    fwc_reference_wrapper_U.FloorSatTicks = 40U;
    fwc_reference_wrapper_step();
    assert(fwc_reference_wrapper_Y.RecoveryAct == 1U);
    fwc_reference_wrapper_U.FloorSatTicks = 0U;
    fwc_reference_wrapper_U.Params[7] = 0.0F;
    fwc_reference_wrapper_U.VutilReq = 0.90F;
    fwc_reference_wrapper_U.VutilAct = 0.90F;
    for (sample = 0U; sample < 39U; ++sample)
    {
        fwc_reference_wrapper_step();
        assert(fwc_reference_wrapper_Y.RecoveryAct == 1U);
    }
    fwc_reference_wrapper_step();
    assert(fwc_reference_wrapper_Y.RecoveryAct == 0U);

    fwc_reference_wrapper_U.Enable = 0U;
    fwc_reference_wrapper_step();
    assert(fwc_reference_wrapper_Y.Active == 0U);
    assert(fwc_reference_wrapper_Y.Status == 0U);
    assert(fwc_reference_wrapper_DW.State_DSTATE[0] == 0.0F);
    assert(fwc_reference_wrapper_DW.State_DSTATE[1] == 0.0F);

    compareProductionAdapter();
    puts("FWC generated-wrapper replay passed");
    return 0;
}
