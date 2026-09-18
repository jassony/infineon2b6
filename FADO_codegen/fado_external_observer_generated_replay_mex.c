#include "mex.h"

#include <string.h>

/* MEX already supplies the compatible fixed-width MATLAB type aliases.
 * Keep the production ERT rtwtypes.h unchanged and skip it in this host-only
 * translation unit to avoid duplicate byte_T and complex type definitions. */
#ifndef RTWTYPES_H
#define RTWTYPES_H
#endif
#include "fado_external_observer_wrapper_ert_rtw/fado_external_observer_wrapper.h"

enum
{
    FADO_CAL_ENABLE = 0,
    FADO_CAL_STATOR_RESISTANCE_OHM,
    FADO_CAL_QUADRATURE_INDUCTANCE_H,
    FADO_CAL_POLE_PAIRS,
    FADO_CAL_FLUX_LIMIT_WB,
    FADO_CAL_KDF_PER_S,
    FADO_CAL_LOW_SPEED_THRESHOLD_HZ,
    FADO_CAL_KAF_RADPS,
    FADO_CAL_FAST_T2S_BANDWIDTH_HZ,
    FADO_CAL_SLOW_T2S_BANDWIDTH_HZ,
    FADO_CAL_T2S_DAMPING,
    FADO_CAL_VOLTAGE_ALPHA_OFFSET_V,
    FADO_CAL_VOLTAGE_BETA_OFFSET_V,
    FADO_CAL_VOLTAGE_PREPROCESS_ENABLE,
    FADO_CAL_COUNT
};

static void localRequireSingleMatrix(const mxArray *array,
                                     const mwSize rows,
                                     const mwSize columns,
                                     const char *name)
{
    if ((!mxIsSingle(array)) || mxIsComplex(array)
        || (mxGetM(array) != rows) || (mxGetN(array) != columns))
    {
        mexErrMsgIdAndTxt("FADO:Replay:Input", "%s must be a real single %lux%lu matrix.",
                          name, (unsigned long)rows, (unsigned long)columns);
    }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    const mwSize *inputDims;
    const real32_T *inputData;
    const real32_T *calibration;
    real32_T *outputData;
    mwSize sample;
    mwSize outputIndex;
    mwSize sampleCount;

    if ((nrhs != 2) || (nlhs != 1))
    {
        mexErrMsgIdAndTxt("FADO:Replay:Arguments",
                          "Use fado_external_observer_generated_replay_mex(samples, calibration).");
    }

    if ((!mxIsSingle(prhs[0])) || mxIsComplex(prhs[0]) || (mxGetN(prhs[0]) != 4))
    {
        mexErrMsgIdAndTxt("FADO:Replay:Samples",
                          "samples must be a real N-by-4 single matrix [Valpha Vbeta Ialpha Ibeta].");
    }

    inputDims = mxGetDimensions(prhs[0]);
    sampleCount = inputDims[0];
    localRequireSingleMatrix(prhs[1], 1, FADO_CAL_COUNT, "calibration");
    inputData = (const real32_T *)mxGetData(prhs[0]);
    calibration = (const real32_T *)mxGetData(prhs[1]);

    plhs[0] = mxCreateNumericMatrix(sampleCount, 18, mxSINGLE_CLASS, mxREAL);
    outputData = (real32_T *)mxGetData(plhs[0]);

    memset(&fado_external_observer_wrapp_DW, 0, sizeof(fado_external_observer_wrapp_DW));
    memset(&fado_external_observer_wrappe_U, 0, sizeof(fado_external_observer_wrappe_U));
    memset(&fado_external_observer_wrappe_Y, 0, sizeof(fado_external_observer_wrappe_Y));
    fado_external_observer_wrapper_initialize();

    fado_external_observer_wrappe_U.fadoEnable =
        (boolean_T)(calibration[FADO_CAL_ENABLE] > 0.5F);
    fado_external_observer_wrappe_U.fadoStatorResistance_Ohm =
        calibration[FADO_CAL_STATOR_RESISTANCE_OHM];
    fado_external_observer_wrappe_U.fadoQuadratureInductance_H =
        calibration[FADO_CAL_QUADRATURE_INDUCTANCE_H];
    fado_external_observer_wrappe_U.fadoPolePairs =
        (uint8_T)calibration[FADO_CAL_POLE_PAIRS];
    fado_external_observer_wrappe_U.fadoFluxLimit_Wb = calibration[FADO_CAL_FLUX_LIMIT_WB];
    fado_external_observer_wrappe_U.fadoKdf_per_s = calibration[FADO_CAL_KDF_PER_S];
    fado_external_observer_wrappe_U.fadoLowSpeedThreshold_Hz =
        calibration[FADO_CAL_LOW_SPEED_THRESHOLD_HZ];
    fado_external_observer_wrappe_U.fadoKaf_radps = calibration[FADO_CAL_KAF_RADPS];
    fado_external_observer_wrappe_U.fadoFastT2SBandwidth_Hz =
        calibration[FADO_CAL_FAST_T2S_BANDWIDTH_HZ];
    fado_external_observer_wrappe_U.fadoSlowT2SBandwidth_Hz =
        calibration[FADO_CAL_SLOW_T2S_BANDWIDTH_HZ];
    fado_external_observer_wrappe_U.fadoT2SDamping = calibration[FADO_CAL_T2S_DAMPING];
    fado_external_observer_wrappe_U.fadoVoltageAlphaOffset_V =
        calibration[FADO_CAL_VOLTAGE_ALPHA_OFFSET_V];
    fado_external_observer_wrappe_U.fadoVoltageBetaOffset_V =
        calibration[FADO_CAL_VOLTAGE_BETA_OFFSET_V];
    fado_external_observer_wrappe_U.fadoVoltagePreprocessEnable =
        (boolean_T)(calibration[FADO_CAL_VOLTAGE_PREPROCESS_ENABLE] > 0.5F);

    for (sample = 0; sample < sampleCount; sample++)
    {
        fado_external_observer_wrappe_U.fadoVoltageAlpha_V = inputData[sample];
        fado_external_observer_wrappe_U.fadoVoltageBeta_V = inputData[sampleCount + sample];
        fado_external_observer_wrappe_U.fadoCurrentAlpha_A = inputData[2 * sampleCount + sample];
        fado_external_observer_wrappe_U.fadoCurrentBeta_A = inputData[3 * sampleCount + sample];
        fado_external_observer_wrapper_step();

        for (outputIndex = 0; outputIndex < 18; outputIndex++)
        {
            outputData[outputIndex * sampleCount + sample] =
                fado_external_observer_wrappe_Y.fadoOutput[outputIndex];
        }
    }
}
