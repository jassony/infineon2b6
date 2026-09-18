function verify_adrc_speed_controller_wrapper
% Compare the wrapper model with the explicit single-precision discrete ADRC.

model = 'adrc_speed_controller_wrapper';
Ts = 5.0e-4;
sampleCount = 4000;
time = (0:(sampleCount - 1))' * Ts;

input = localReplayInputs(time);
inputData = Simulink.SimulationData.Dataset;
inputData = inputData.addElement(timeseries(input.reference, time), ...
    'Speed_Reference_PU');
inputData = inputData.addElement(timeseries(input.feedback, time), ...
    'Speed_Feedback_PU');
inputData = inputData.addElement(timeseries(input.criticalGain, time), ...
    'Critical_Gain_PU_per_PU_s2');
inputData = inputData.addElement(timeseries(input.controlBandwidth, time), ...
    'Control_Bandwidth_radps');
inputData = inputData.addElement(timeseries(input.observerBandwidth, time), ...
    'Observer_Bandwidth_radps');
inputData = inputData.addElement(timeseries(input.upperLimit, time), ...
    'Iq_Upper_Limit_PU');
inputData = inputData.addElement(timeseries(input.lowerLimit, time), ...
    'Iq_Lower_Limit_PU');
inputData = inputData.addElement(timeseries(input.reset, time), 'Reset_u8');

simulationInput = Simulink.SimulationInput(model);
simulationInput = simulationInput.setExternalInput(inputData);
simulationInput = simulationInput.setModelParameter( ...
    'StopTime', num2str(time(end), 17), ...
    'SimulationMode', 'normal', ...
    'SaveOutput', 'on', ...
    'SaveFormat', 'Dataset', ...
    'OutputSaveName', 'yout');
simulationOutput = sim(simulationInput);

modelOutput = localDatasetOutput(simulationOutput.yout);
referenceOutput = localAdrcReference(input);
localCompareOutput(modelOutput, referenceOutput);

fprintf(['ADRC wrapper replay passed. Includes step, reversal, noise, reset, ' ...
    'live tuning, parameter-invalid, and non-finite input vectors.\n']);
end

function input = localReplayInputs(time)
count = numel(time);
input.reference = zeros(count, 1, 'single');
input.reference(time >= 0.050) = single(0.25);
input.reference(time >= 0.400) = single(0.55);
input.reference(time >= 1.000) = single(-0.30);
input.feedback = single(0.68) * input.reference + single(0.015) * ...
    sin(single(2.0 * pi * 7.0) * single(time));

input.criticalGain = single(19817.6773682124) * ones(count, 1, 'single');
input.controlBandwidth = single(104.71975511966) * ones(count, 1, 'single');
input.observerBandwidth = single(837.758040957278) * ones(count, 1, 'single');
input.upperLimit = single(0.8) * ones(count, 1, 'single');
input.lowerLimit = single(-0.8) * ones(count, 1, 'single');
input.reset = zeros(count, 1, 'uint8');

% Live-tuning vectors do not reset observer states.
liveTuning = time >= 0.700;
input.criticalGain(liveTuning) = single(17000.0);
input.controlBandwidth(liveTuning) = single(90.0);
input.observerBandwidth(liveTuning) = single(720.0);
input.upperLimit(liveTuning) = single(0.65);
input.lowerLimit(liveTuning) = single(-0.60);

% Reset, invalid parameter, invalid saturation range, and non-finite input.
input.reset((time >= 1.200) & (time < 1.205)) = uint8(1);
input.criticalGain((time >= 1.350) & (time < 1.355)) = single(0);
input.controlBandwidth((time >= 1.450) & (time < 1.455)) = single(-1);
input.upperLimit((time >= 1.550) & (time < 1.555)) = single(-0.2);
input.lowerLimit((time >= 1.550) & (time < 1.555)) = single(0.2);
input.feedback(round(1.650 / 5.0e-4) + 1) = single(NaN);
end

function output = localDatasetOutput(dataset)
output.iqReference = single(dataset.getElement(1).Values.Data(:));
output.valid = uint8(dataset.getElement(2).Values.Data(:));
output.status = uint8(dataset.getElement(3).Values.Data(:));
output.referenceFiltered = single(dataset.getElement(4).Values.Data(:));
output.estimatedSpeed = single(dataset.getElement(5).Values.Data(:));
output.estimatedAcceleration = single(dataset.getElement(6).Values.Data(:));
output.estimatedDisturbance = single(dataset.getElement(7).Values.Data(:));
end

function output = localAdrcReference(input)
Ts = single(5.0e-4);
filterGain = single(0.1);
count = numel(input.reference);
output.iqReference = zeros(count, 1, 'single');
output.valid = zeros(count, 1, 'uint8');
output.status = zeros(count, 1, 'uint8');
output.referenceFiltered = zeros(count, 1, 'single');
output.estimatedSpeed = zeros(count, 1, 'single');
output.estimatedAcceleration = zeros(count, 1, 'single');
output.estimatedDisturbance = zeros(count, 1, 'single');
referenceFilterState = single(0);
state0 = single(0);
state1 = single(0);
state2 = single(0);

% The Chart entry action executes the first ADRC step from the zero initial
% state, matching the generated initialize/step sequence and the adapter.
for index = 1:count
    [output.iqReference(index), output.valid(index), output.status(index), ...
        output.referenceFiltered(index), referenceFilterState, state0, state1, state2] = ...
        localAdrcStep(input.reference(index), input.feedback(index), ...
        input.criticalGain(index), input.controlBandwidth(index), ...
        input.observerBandwidth(index), input.upperLimit(index), ...
        input.lowerLimit(index), input.reset(index), referenceFilterState, ...
        state0, state1, state2, Ts, filterGain);
    output.estimatedSpeed(index) = state0;
    output.estimatedAcceleration(index) = state1;
    output.estimatedDisturbance(index) = state2;
end
end

function [iqReference, valid, status, referenceFiltered, nextFilterState, ...
    nextState0, nextState1, nextState2] = localAdrcStep(reference, feedback, ...
    criticalGain, controlBandwidth, observerBandwidth, upperLimit, lowerLimit, ...
    reset, filterState, state0, state1, state2, Ts, filterGain)
iqReference = single(0);
valid = uint8(0);
status = uint8(0);
referenceFiltered = single(0);
nextFilterState = single(0);
nextState0 = single(0);
nextState1 = single(0);
nextState2 = single(0);

if reset ~= uint8(0)
    return;
end

if ~(isfinite(reference) && isfinite(feedback) && isfinite(criticalGain) ...
        && isfinite(controlBandwidth) && isfinite(observerBandwidth) ...
        && isfinite(upperLimit) && isfinite(lowerLimit))
    status = uint8(3);
    return;
end

if (criticalGain <= single(0)) || (controlBandwidth <= single(0)) ...
        || (observerBandwidth <= single(0)) || (lowerLimit > upperLimit)
    status = uint8(2);
    return;
end

observerPole = exp(-observerBandwidth * Ts);
oneMinusPole = single(1) - observerPole;
observerGain0 = single(1) - observerPole * observerPole * observerPole;
observerGain1 = (single(1.5) / Ts) * oneMinusPole * oneMinusPole ...
    * (single(1) + observerPole);
observerGain2 = oneMinusPole * oneMinusPole * oneMinusPole / (Ts * Ts);
controlGain = controlBandwidth * controlBandwidth;
dampingGain = single(2) * controlBandwidth;
inputGain0 = single(0.5) * Ts * Ts * criticalGain;
inputGain1 = Ts * criticalGain;
inverseCriticalGain = single(1) / criticalGain;

if ~(isfinite(observerPole) && isfinite(observerGain0) && isfinite(observerGain1) ...
        && isfinite(observerGain2) && isfinite(controlGain) && isfinite(dampingGain) ...
        && isfinite(inputGain0) && isfinite(inputGain1) && isfinite(inverseCriticalGain))
    status = uint8(3);
    return;
end

referenceFiltered = filterState + filterGain * (reference - filterState);
observerError = feedback - state0;
correctedState0 = state0 + observerGain0 * observerError;
correctedState1 = state1 + observerGain1 * observerError;
correctedState2 = state2 + observerGain2 * observerError;
iqReference = ((referenceFiltered - correctedState0) * controlGain ...
    - dampingGain * correctedState1 - correctedState2) * inverseCriticalGain;
iqReference = min(max(iqReference, lowerLimit), upperLimit);

nextFilterState = referenceFiltered;
nextState0 = correctedState0 + Ts * correctedState1 ...
    + single(0.5) * Ts * Ts * correctedState2 + inputGain0 * iqReference;
nextState1 = correctedState1 + Ts * correctedState2 + inputGain1 * iqReference;
nextState2 = correctedState2;

if ~(isfinite(referenceFiltered) && isfinite(iqReference) && isfinite(nextState0) ...
        && isfinite(nextState1) && isfinite(nextState2))
    iqReference = single(0);
    referenceFiltered = single(0);
    nextFilterState = single(0);
    nextState0 = single(0);
    nextState1 = single(0);
    nextState2 = single(0);
    status = uint8(3);
    return;
end

valid = uint8(1);
status = uint8(1);
end

function localCompareOutput(modelOutput, referenceOutput)
singleSignals = {'iqReference', 'referenceFiltered', 'estimatedSpeed', ...
    'estimatedAcceleration', 'estimatedDisturbance'};
for index = 1:numel(singleSignals)
    name = singleSignals{index};
    modelSignal = modelOutput.(name);
    referenceSignal = referenceOutput.(name);
    if numel(modelSignal) ~= numel(referenceSignal)
        error('ADRC:ReplayLength', '%s output length differs.', name);
    end

    maximumError = max(abs(modelSignal - referenceSignal));
    if maximumError > single(2.0e-5)
        error('ADRC:ReplayMismatch', ...
            '%s mismatch: maximum absolute PU error is %.9g.', name, maximumError);
    end
end

if ~isequal(modelOutput.valid, referenceOutput.valid)
    error('ADRC:ReplayValidity', 'ADRC wrapper valid output differs from reference.');
end

if ~isequal(modelOutput.status, referenceOutput.status)
    error('ADRC:ReplayStatus', 'ADRC wrapper status output differs from reference.');
end
end
