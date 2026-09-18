function results = verify_vafid_external_observer_wrapper()
%VERIFY_VAFID_EXTERNAL_OBSERVER_WRAPPER Build and replay MEX and ERT code.

folder = fileparts(mfilename('fullpath'));
modelName = 'vafid_external_observer_wrapper';
modelFile = fullfile(folder, [modelName '.slx']);
ertFolder = fullfile(folder, [modelName '_ert_rtw']);
addpath(folder);

screenerResult = coder.screener('vafid_external_observer_wrapper_step');
assert(isempty(screenerResult.UnsupportedCalls), ...
    'VAFID wrapper contains unsupported code-generation calls.');
assert(isempty(screenerResult.Messages), ...
    'VAFID wrapper contains code-generation readiness messages.');

inputTypes = {uint8(0), false, false, single(0), single(0), ...
    single(0), single(0), single(0), single(0), single(0), ...
    coder.typeof(zeros(30, 1, 'single'))};
mexFolder = tempname;
mkdir(mexFolder);
mexConfig = coder.config('mex');
mexConfig.TargetLang = 'C';
mexConfig.IntegrityChecks = true;
mexConfig.EnableDynamicMemoryAllocation = false;
originalFolder = pwd;
folderCleanup = onCleanup(@() cd(originalFolder));
cd(mexFolder);
codegen('vafid_external_observer_wrapper_step', '-config', mexConfig, ...
    '-args', inputTypes, '-d', mexFolder);
cd(originalFolder);
addpath(mexFolder);

open_system(modelFile);
set_param(modelName, 'SimulationCommand', 'update');
cacheFolder = tempname;
mkdir(cacheFolder);
Simulink.fileGenControl('set', 'CacheFolder', cacheFolder, ...
    'CodeGenFolder', folder, 'createDir', true);
slbuild(modelName);

replayMexFolder = tempname;
mkdir(replayMexFolder);
lccOptions = fullfile(matlabroot, 'rtw', 'c', 'tools', 'lcc-win64.xml');
harnessFile = localWriteReplayHarness(replayMexFolder);
mex('-f', lccOptions, '-R2018a', ['-I' ertFolder], ...
    '-outdir', replayMexFolder, '-output', 'vafid_ert_replay_mex', ...
    harnessFile, ...
    fullfile(ertFolder, 'vafid_external_observer_wrapper.c'));
addpath(replayMexFolder);

[vectors, configValues] = localReplayVectors();
clear vafid_discrete_reference_step vafid_external_observer_wrapper_step;
matlabTrace = localRunScalarEntry( ...
    'vafid_external_observer_wrapper_step', vectors, configValues);
clear vafid_external_observer_wrapper_step_mex;
mexTrace = localRunScalarEntry( ...
    'vafid_external_observer_wrapper_step_mex', vectors, configValues);
ertTrace = localRunErtEntry(vectors, configValues);

mexMaximumDifference = localCompareTraces(matlabTrace, mexTrace);
ertMaximumDifference = localCompareTraces(matlabTrace, ertTrace);
localVerifySequence(matlabTrace);

results = struct( ...
    'screenerUnsupportedCalls', numel(screenerResult.UnsupportedCalls), ...
    'screenerMessages', numel(screenerResult.Messages), ...
    'mexMaximumSingleDifference', mexMaximumDifference, ...
    'ertMaximumSingleDifference', ertMaximumDifference, ...
    'sampleCount', uint32(numel(vectors.mode)), ...
    'fixedStep_s', single(5e-5));
fprintf(['VAFID wrapper replay passed: %u samples, MEX max diff %.9g, ' ...
    'ERT max diff %.9g.\n'], results.sampleCount, ...
    results.mexMaximumSingleDifference, ...
    results.ertMaximumSingleDifference);
end

function [vectors, configValues] = localReplayVectors()
config = vafid_default_config();
config.settleTime_s = single(2) * config.sampleTime_s;
config.windowLength_samples = uint32(8);
config.requiredAcceptedWindows = uint16(1);
configValues = vafid_pack_wrapper_config(config);

sampleCount = 24;
sampleIndex = single((0:(sampleCount - 1)).');
time_s = sampleIndex * single(5e-5);
vectors.mode = uint8(ones(sampleCount, 1));
vectors.mode(1:3) = uint8(0);
vectors.resetRequest = false(sampleCount, 1);
vectors.resetRequest(4) = true;
vectors.sampleValid = true(sampleCount, 1);
vectors.voltageAlpha_V = single(0.5) * ...
    sin(single(2 * pi * 150) * time_s);
vectors.voltageBeta_V = single(0.2) * ...
    cos(single(2 * pi * 220) * time_s);
vectors.currentAlpha_A = single(0.1) * ...
    sin(single(2 * pi * 150) * time_s);
vectors.currentBeta_A = single(0.1) * ...
    cos(single(2 * pi * 220) * time_s);
vectors.kreElectricalAngle_rad = single(0.05) * sampleIndex;
vectors.kreElectricalOmega_radps = single(100) * ones(sampleCount, 1, 'single');
vectors.kreActiveFlux_Wb = single(0.046) * ones(sampleCount, 1, 'single');
vectors.voltageAlpha_V(16) = single(NaN);
end

function trace = localRunScalarEntry(entryPoint, vectors, configValues)
sampleCount = numel(vectors.mode);
trace = localPreallocateTrace(sampleCount);
for sampleIndex = 1:sampleCount
    [trace.probeD_PU(sampleIndex), trace.probeQ_PU(sampleIndex), ...
        trace.rs_Ohm(sampleIndex), trace.ld_H(sampleIndex), ...
        trace.lq_H(sampleIndex), trace.fluxPM_Wb(sampleIndex), ...
        trace.estimateValid(sampleIndex), trace.freshEstimate(sampleIndex), ...
        trace.staleEstimate(sampleIndex), trace.status(sampleIndex), ...
        trace.conditionNumber(sampleIndex), ...
        trace.relativeResidual(sampleIndex), ...
        trace.consecutiveAcceptedWindows(sampleIndex), ...
        trace.windowSampleCount(sampleIndex)] = feval(entryPoint, ...
        vectors.mode(sampleIndex), vectors.resetRequest(sampleIndex), ...
        vectors.sampleValid(sampleIndex), ...
        vectors.voltageAlpha_V(sampleIndex), ...
        vectors.voltageBeta_V(sampleIndex), ...
        vectors.currentAlpha_A(sampleIndex), ...
        vectors.currentBeta_A(sampleIndex), ...
        vectors.kreElectricalAngle_rad(sampleIndex), ...
        vectors.kreElectricalOmega_radps(sampleIndex), ...
        vectors.kreActiveFlux_Wb(sampleIndex), configValues);
end
end

function trace = localRunErtEntry(vectors, configValues)
[trace.probeD_PU, trace.probeQ_PU, trace.rs_Ohm, trace.ld_H, ...
    trace.lq_H, trace.fluxPM_Wb, trace.estimateValid, ...
    trace.freshEstimate, trace.staleEstimate, trace.status, ...
    trace.conditionNumber, trace.relativeResidual, ...
    trace.consecutiveAcceptedWindows, trace.windowSampleCount] = ...
    vafid_ert_replay_mex(vectors.mode, vectors.resetRequest, ...
    vectors.sampleValid, vectors.voltageAlpha_V, ...
    vectors.voltageBeta_V, vectors.currentAlpha_A, ...
    vectors.currentBeta_A, vectors.kreElectricalAngle_rad, ...
    vectors.kreElectricalOmega_radps, vectors.kreActiveFlux_Wb, ...
    configValues);
end

function trace = localPreallocateTrace(sampleCount)
singleOutput = zeros(sampleCount, 1, 'single');
logicalOutput = false(sampleCount, 1);
trace = struct( ...
    'probeD_PU', singleOutput, ...
    'probeQ_PU', singleOutput, ...
    'rs_Ohm', singleOutput, ...
    'ld_H', singleOutput, ...
    'lq_H', singleOutput, ...
    'fluxPM_Wb', singleOutput, ...
    'estimateValid', logicalOutput, ...
    'freshEstimate', logicalOutput, ...
    'staleEstimate', logicalOutput, ...
    'status', zeros(sampleCount, 1, 'uint8'), ...
    'conditionNumber', singleOutput, ...
    'relativeResidual', singleOutput, ...
    'consecutiveAcceptedWindows', zeros(sampleCount, 1, 'uint16'), ...
    'windowSampleCount', zeros(sampleCount, 1, 'uint32'));
end

function maximumDifference = localCompareTraces(reference, candidate)
exactFields = {'estimateValid', 'freshEstimate', 'staleEstimate', ...
    'status', 'consecutiveAcceptedWindows', 'windowSampleCount'};
for fieldIndex = 1:numel(exactFields)
    fieldName = exactFields{fieldIndex};
    assert(isequal(reference.(fieldName), candidate.(fieldName)), ...
        'Exact output mismatch for %s.', fieldName);
end

singleFields = {'probeD_PU', 'probeQ_PU', 'rs_Ohm', 'ld_H', 'lq_H', ...
    'fluxPM_Wb', 'conditionNumber', 'relativeResidual'};
maximumDifference = 0;
for fieldIndex = 1:numel(singleFields)
    fieldName = singleFields{fieldIndex};
    referenceValue = reference.(fieldName);
    candidateValue = candidate.(fieldName);
    finiteMask = isfinite(referenceValue);
    assert(isequal(finiteMask, isfinite(candidateValue)), ...
        'Finite-value classification mismatch for %s.', fieldName);
    if any(finiteMask)
        fieldDifference = max(abs( ...
            referenceValue(finiteMask) - candidateValue(finiteMask)));
        maximumDifference = max(maximumDifference, double(fieldDifference));
    end
end
assert(maximumDifference <= 2e-6, ...
    'Single-precision replay difference exceeds tolerance.');
end

function localVerifySequence(trace)
assert(all(trace.status(1:3) == uint8(0)), ...
    'Off mode must remain inert.');
assert(trace.status(4) == uint8(1), ...
    'Reset sample must publish reset status.');
assert(all(trace.status(5:6) == uint8(2)), ...
    'Two configured settling samples are required.');
assert(all(trace.status(7:13) == uint8(3)), ...
    'Window must collect exactly seven partial samples.');
assert(trace.windowSampleCount(13) == uint32(7) && ...
    trace.windowSampleCount(14) == uint32(0), ...
    'Eight-sample window must complete and clear on sample 14.');
assert(trace.status(16) == uint8(8) && ...
    trace.windowSampleCount(16) == uint32(0), ...
    'Non-finite input must reject and clear the window immediately.');
end

function harnessFile = localWriteReplayHarness(outputFolder)
harnessFile = fullfile(outputFolder, 'vafid_ert_replay_harness.c');
lines = {
    '#include "mex.h"'
    '#include "vafid_external_observer_wrapper.h"'
    '#include <math.h>'
    '#include <string.h>'
    'real_T rtNaN, rtInf, rtMinusInf;'
    'real32_T rtNaNF, rtInfF, rtMinusInfF;'
    'float fmodf(float x, float y) { return (float)fmod((double)x, (double)y); }'
    'float fmaxf(float x, float y) {'
    '  if (isnan(x)) return y;'
    '  if (isnan(y)) return x;'
    '  return (x > y) ? x : y;'
    '}'
    'boolean_T rtIsInf(real_T x) { return (boolean_T)isinf(x); }'
    'boolean_T rtIsInfF(real32_T x) { return (boolean_T)isinf(x); }'
    'boolean_T rtIsNaN(real_T x) { return (boolean_T)(isnan(x) != 0); }'
    'boolean_T rtIsNaNF(real32_T x) { return (boolean_T)(isnan(x) != 0); }'
    'void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {'
    '  mwSize n, k;'
    '  const uint8_T *mode; const mxLogical *resetRequest, *sampleValid;'
    '  const real32_T *va, *vb, *ia, *ib, *angle, *omega, *flux, *config;'
    '  real32_T *probeD, *probeQ, *rs, *ld, *lq, *fluxPM, *condition, *residual;'
    '  mxLogical *valid, *fresh, *stale; uint8_T *status;'
    '  uint16_T *accepted; uint32_T *window;'
    '  if (nrhs != 11 || nlhs != 14) mexErrMsgIdAndTxt("VAFID:Replay:Arity", "Expected 11 inputs and 14 outputs");'
    '  n = mxGetNumberOfElements(prhs[0]);'
    '  mode = (const uint8_T *)mxGetData(prhs[0]);'
    '  resetRequest = (const mxLogical *)mxGetData(prhs[1]);'
    '  sampleValid = (const mxLogical *)mxGetData(prhs[2]);'
    '  va = (const real32_T *)mxGetData(prhs[3]); vb = (const real32_T *)mxGetData(prhs[4]);'
    '  ia = (const real32_T *)mxGetData(prhs[5]); ib = (const real32_T *)mxGetData(prhs[6]);'
    '  angle = (const real32_T *)mxGetData(prhs[7]); omega = (const real32_T *)mxGetData(prhs[8]);'
    '  flux = (const real32_T *)mxGetData(prhs[9]); config = (const real32_T *)mxGetData(prhs[10]);'
    '  plhs[0] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); probeD = (real32_T *)mxGetData(plhs[0]);'
    '  plhs[1] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); probeQ = (real32_T *)mxGetData(plhs[1]);'
    '  plhs[2] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); rs = (real32_T *)mxGetData(plhs[2]);'
    '  plhs[3] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); ld = (real32_T *)mxGetData(plhs[3]);'
    '  plhs[4] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); lq = (real32_T *)mxGetData(plhs[4]);'
    '  plhs[5] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); fluxPM = (real32_T *)mxGetData(plhs[5]);'
    '  plhs[6] = mxCreateLogicalMatrix(n,1); valid = (mxLogical *)mxGetData(plhs[6]);'
    '  plhs[7] = mxCreateLogicalMatrix(n,1); fresh = (mxLogical *)mxGetData(plhs[7]);'
    '  plhs[8] = mxCreateLogicalMatrix(n,1); stale = (mxLogical *)mxGetData(plhs[8]);'
    '  plhs[9] = mxCreateNumericMatrix(n,1,mxUINT8_CLASS,mxREAL); status = (uint8_T *)mxGetData(plhs[9]);'
    '  plhs[10] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); condition = (real32_T *)mxGetData(plhs[10]);'
    '  plhs[11] = mxCreateNumericMatrix(n,1,mxSINGLE_CLASS,mxREAL); residual = (real32_T *)mxGetData(plhs[11]);'
    '  plhs[12] = mxCreateNumericMatrix(n,1,mxUINT16_CLASS,mxREAL); accepted = (uint16_T *)mxGetData(plhs[12]);'
    '  plhs[13] = mxCreateNumericMatrix(n,1,mxUINT32_CLASS,mxREAL); window = (uint32_T *)mxGetData(plhs[13]);'
    '  rtNaN = mxGetNaN(); rtInf = mxGetInf(); rtMinusInf = -mxGetInf();'
    '  rtNaNF = (real32_T)rtNaN; rtInfF = (real32_T)rtInf; rtMinusInfF = (real32_T)rtMinusInf;'
    '  memset(&vafid_external_observer_wrapp_U,0,sizeof(vafid_external_observer_wrapp_U));'
    '  memset(&vafid_external_observer_wrapp_Y,0,sizeof(vafid_external_observer_wrapp_Y));'
    '  memset(&vafid_external_observer_wrap_DW,0,sizeof(vafid_external_observer_wrap_DW));'
    '  vafid_external_observer_wrapper_initialize();'
    '  for (k=0; k<n; ++k) {'
    '    vafid_external_observer_wrapp_U.mode=mode[k];'
    '    vafid_external_observer_wrapp_U.resetRequest=(boolean_T)resetRequest[k];'
    '    vafid_external_observer_wrapp_U.sampleValid=(boolean_T)sampleValid[k];'
    '    vafid_external_observer_wrapp_U.voltageAlpha_V=va[k]; vafid_external_observer_wrapp_U.voltageBeta_V=vb[k];'
    '    vafid_external_observer_wrapp_U.currentAlpha_A=ia[k]; vafid_external_observer_wrapp_U.currentBeta_A=ib[k];'
    '    vafid_external_observer_wrapp_U.kreElectricalAngle_rad=angle[k];'
    '    vafid_external_observer_wrapp_U.kreElectricalOmega_radps=omega[k];'
    '    vafid_external_observer_wrapp_U.kreActiveFlux_Wb=flux[k];'
    '    memcpy(vafid_external_observer_wrapp_U.configValues,config,sizeof(vafid_external_observer_wrapp_U.configValues));'
    '    vafid_external_observer_wrapper_step();'
    '    probeD[k]=vafid_external_observer_wrapp_Y.probeD_PU; probeQ[k]=vafid_external_observer_wrapp_Y.probeQ_PU;'
    '    rs[k]=vafid_external_observer_wrapp_Y.rs_Ohm; ld[k]=vafid_external_observer_wrapp_Y.ld_H;'
    '    lq[k]=vafid_external_observer_wrapp_Y.lq_H; fluxPM[k]=vafid_external_observer_wrapp_Y.fluxPM_Wb;'
    '    valid[k]=(mxLogical)vafid_external_observer_wrapp_Y.estimateValid;'
    '    fresh[k]=(mxLogical)vafid_external_observer_wrapp_Y.freshEstimate;'
    '    stale[k]=(mxLogical)vafid_external_observer_wrapp_Y.staleEstimate; status[k]=vafid_external_observer_wrapp_Y.status;'
    '    condition[k]=vafid_external_observer_wrapp_Y.conditionNumber; residual[k]=vafid_external_observer_wrapp_Y.relativeResidual;'
    '    accepted[k]=vafid_external_observer_wrapp_Y.consecutiveAcceptedWindows; window[k]=vafid_external_observer_wrapp_Y.windowSampleCount;'
    '  }'
    '  vafid_external_observer_wrapper_terminate();'
    '}'
    };
fileIdentifier = fopen(harnessFile, 'wt');
assert(fileIdentifier >= 0, 'Unable to create the temporary ERT replay harness.');
fileCleanup = onCleanup(@() fclose(fileIdentifier));
fprintf(fileIdentifier, '%s\n', lines{:});
end
