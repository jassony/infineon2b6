function results = verify_kre_discrete_observer_wrapper()
%VERIFY_KRE_DISCRETE_OBSERVER_WRAPPER Replay pure blocks against KRE.

testDir = fileparts(mfilename('fullpath'));
repoDir = fileparts(testDir);
referenceDir = fullfile(repoDir, 'KRE_codegen');
sourceDir = fullfile(repoDir, 'ges', 'SensorlessFocFOSMOExample');
originalPath = path;
pathCleanup = onCleanup(@() path(originalPath)); %#ok<NASGU>
addpath(testDir, referenceDir, sourceDir);

Ts = 50e-6;
duration = 1.0;
t = (0:Ts:duration).';
sampleCount = numel(t);

pmFlux = 0.05;
baseVoltage = 100.0;
baseSpeedRpm = 6000.0;
polePairs = 4.0;
params = single([0.5, 450e-6, 550e-6, pmFlux, Ts, baseVoltage, ...
    50.0, baseSpeedRpm, polePairs, 1256.637061, 125.663704, ...
    1.0, 1e-5, 100.0]);
pllParams = single([50.0, 0.70710678]);

mechanicalSpeedRpm = 1000.0;
electricalOmega = mechanicalSpeedRpm * polePairs * 2.0 * pi / 60.0;
electricalAngle = 0.35 + electricalOmega * t;
voltageAlpha = -electricalOmega * pmFlux .* sin(electricalAngle);
voltageBeta = electricalOmega * pmFlux .* cos(electricalAngle);

viFb = single([voltageAlpha / baseVoltage, voltageBeta / baseVoltage, ...
    zeros(sampleCount, 2)]);
enable = zeros(sampleCount, 1, 'uint8');
enable(3:end) = uint8(1);
reset = zeros(sampleCount, 1, 'uint8');

ds = Simulink.SimulationData.Dataset;
ds{1} = timeseries(enable, t);
ds{2} = timeseries(viFb, t);
ds{3} = timeseries(repmat(params, sampleCount, 1), t);
ds{4} = timeseries(reset, t);
ds{5} = timeseries(repmat(pllParams, sampleCount, 1), t);

[referencePosition, referenceSpeed, referenceStatus] = ...
    runProtectedReference(enable, viFb, params, pllParams);
discreteOut = runModel('kre_discrete_observer_wrapper', ds, duration);
discretePosition = signalData(discreteOut, 1);
discreteSpeed = signalData(discreteOut, 2);
discreteFluxMagnitude = signalData(discreteOut, 3);
discreteStatus = signalData(discreteOut, 4);

positionError = mod(double(discretePosition) - double(referencePosition) ...
    + 0.5, 1.0) - 0.5;
speedError = double(discreteSpeed) - double(referenceSpeed);

results = struct;
results.MaxPositionErrorPU = max(abs(positionError));
results.MaxSpeedErrorPU = max(abs(speedError));
results.StatusMismatchCount = nnz(discreteStatus ~= referenceStatus);
results.AllOutputsFinite = all(isfinite(discretePosition)) ...
    && all(isfinite(discreteSpeed)) && all(isfinite(discreteFluxMagnitude));
results.MinFluxMagnitudePU = min(double(discreteFluxMagnitude));
results.MaxFluxMagnitudePU = max(double(discreteFluxMagnitude));

assert(results.AllOutputsFinite, 'KRE discrete outputs contain NaN or Inf.');
assert(results.MaxPositionErrorPU <= 1e-6, ...
    'Position replay error exceeds 1e-6 PU.');
assert(results.MaxSpeedErrorPU <= 1e-6, ...
    'Speed replay error exceeds 1e-6 PU.');
assert(results.StatusMismatchCount == 0, ...
    'Status output differs from the protected reference wrapper.');
assert(results.MinFluxMagnitudePU >= 0.0, ...
    'Normalized flux magnitude must be nonnegative.');

fprintf('KRE discrete replay passed.\n');
fprintf('  max position error: %.9g PU\n', results.MaxPositionErrorPU);
fprintf('  max speed error:    %.9g PU\n', results.MaxSpeedErrorPU);
fprintf('  status mismatches:  %d\n', results.StatusMismatchCount);
fprintf('  flux magnitude:     [%.9g, %.9g] PU\n', ...
    results.MinFluxMagnitudePU, results.MaxFluxMagnitudePU);
end

function out = runModel(modelName, ds, duration)
in = Simulink.SimulationInput(modelName);
in = in.setExternalInput(ds);
in = in.setModelParameter('StopTime', num2str(duration, '%.17g'));
in = in.setModelParameter('SaveOutput', 'on');
in = in.setModelParameter('OutputSaveName', 'yout');
in = in.setModelParameter('SaveFormat', 'Dataset');
out = sim(in);
end

function data = signalData(out, elementIndex)
element = out.yout.getElement(elementIndex);
data = element.Values.Data;
end

function [position, speed, status] = runProtectedReference( ...
    enable, viFb, params, pllParams)
clear kre_external_observer_step
clear mcb_pmsm_foc_sensorless_f28379d_kre_observer_step
sampleCount = size(viFb, 1);
position = zeros(sampleCount, 1, 'single');
speed = zeros(sampleCount, 1, 'single');
status = zeros(sampleCount, 1, 'uint8');
for sampleIndex = 1:sampleCount
    [position(sampleIndex), speed(sampleIndex), ~, status(sampleIndex)] = ...
        kre_external_observer_step(enable(sampleIndex), ...
        viFb(sampleIndex, :), params, pllParams);
end
end
