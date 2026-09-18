function result = verify_kre_external_observer_wrapper()
%VERIFY_KRE_EXTERNAL_OBSERVER_WRAPPER Run non-graphical wrapper checks.
%   The first case checks the invalid-parameter guard. The second case uses
%   the project loop period and a reset pulse with a valid parameter vector.

thisDir = fileparts(mfilename('fullpath'));
projectDir = fileparts(thisDir);
modelFile = fullfile(thisDir, 'kre_external_observer_wrapper.slx');
mdl = 'kre_external_observer_wrapper';

addpath(thisDir);
addpath(fullfile(projectDir, 'ges', 'SensorlessFocFOSMOExample'));
load_system(modelFile);
cleanupObj = onCleanup(@() close_system(mdl, 0)); %#ok<NASGU>

sampleTime = 5e-5;
time = (0:sampleTime:10*sampleTime).';
sampleCount = numel(time);

invalidParams = single(zeros(sampleCount, 14));
invalidPllParams = single(zeros(sampleCount, 2));
invalidReset = zeros(sampleCount, 1, 'uint8');
invalidReset(1) = uint8(1);
invalidResult = localRunCase(mdl, time, invalidParams, invalidPllParams, ...
    invalidReset);
invalidStatus = localOutput(invalidResult, 4);
invalidPosition = localOutput(invalidResult, 1);
invalidSpeed = localOutput(invalidResult, 2);
assert(invalidStatus.Data(end) == uint8(2), ...
    'KRE invalid parameter guard did not report status 2.');
assert(all(isfinite(double(invalidPosition.Data))) && ...
       all(isfinite(double(invalidSpeed.Data))), ...
    'KRE invalid parameter guard propagated a non-finite estimate.');

validParams = single([ ...
    0.5, 1e-3, 1e-3, 0.05, sampleTime, 1000, 50, 8500, ...
    4, 100, 1000, 10, 0.001, 20]);
validParams = repmat(validParams, sampleCount, 1);
validPllParams = single(repmat([50.0, 0.70710678], sampleCount, 1));
resetSignal = zeros(sampleCount, 1, 'uint8');
resetSignal(1) = uint8(1);
validResult = localRunCase(mdl, time, validParams, validPllParams, resetSignal);
validStatus = localOutput(validResult, 4);
validPosition = localOutput(validResult, 1);
validSpeed = localOutput(validResult, 2);
assert(validStatus.Data(end) == uint8(1), ...
    'KRE valid parameter case did not report status 1.');
assert(all(isfinite(double(validPosition.Data))) && ...
       all(isfinite(double(validSpeed.Data))), ...
    'KRE valid parameter case produced a non-finite estimate.');

result.invalidStatus = invalidStatus.Data(end);
result.validStatus = validStatus.Data(end);
result.validPositionPU = validPosition.Data(end);
result.validSpeedPU = validSpeed.Data(end);
end

function simulationOutput = localRunCase(mdl, time, params, pllParams, resetSignal)
sampleCount = numel(time);
dataset = Simulink.SimulationData.Dataset;
dataset = dataset.addElement( ...
    timeseries(uint8(ones(sampleCount, 1)), time), 'kreEnable');
dataset = dataset.addElement( ...
    timeseries(single(zeros(sampleCount, 4)), time), 'kreViFb');
dataset = dataset.addElement( ...
    timeseries(params, time), 'kreParams');
dataset = dataset.addElement( ...
    timeseries(resetSignal, time), 'kreReset');
dataset = dataset.addElement( ...
    timeseries(pllParams, time), 'krePllParams');

simulationInput = Simulink.SimulationInput(mdl);
simulationInput = simulationInput.setExternalInput(dataset);
simulationInput = simulationInput.setModelParameter( ...
    'StopTime', num2str(time(end)), ...
    'SaveOutput', 'on', ...
    'OutputSaveName', 'yout');
simulationOutput = sim(simulationInput);
end

function signal = localOutput(simulationOutput, index)
outputs = simulationOutput.yout;
signal = outputs.get(index).Values;
end
