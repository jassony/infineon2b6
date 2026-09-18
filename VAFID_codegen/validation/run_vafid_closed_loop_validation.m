function results = run_vafid_closed_loop_validation(scenarioFilter, throwOnFailure)
%RUN_VAFID_CLOSED_LOOP_VALIDATION Execute strict closed-loop VAFID MIL cases.
%   RESULTS = RUN_VAFID_CLOSED_LOOP_VALIDATION() simulates the independent
%   validation model with a 3x3 speed/load grid plus variable-load,
%   variable-speed, and reversal cases. Inputs are applied with
%   Simulink.SimulationInput and all pass/fail metrics come from model output.
%
%   RESULTS = RUN_VAFID_CLOSED_LOOP_VALIDATION(FILTER, THROWONFAILURE) limits
%   execution to scenario names containing FILTER. THROWONFAILURE defaults to
%   true and raises an error if any row fails.

if nargin < 1
    scenarioFilter = "";
end
if nargin < 2
    throwOnFailure = true;
end

validationFolder = fileparts(mfilename('fullpath'));
algorithmFolder = fileparts(validationFolder);
originalPath = path;
pathCleanup = onCleanup(@() path(originalPath)); %#ok<NASGU>
addpath(algorithmFolder);
addpath(validationFolder);

modelName = 'vafid_closed_loop_validation';
modelFile = fullfile(algorithmFolder, [modelName '.slx']);
assert(isfile(modelFile), 'VAFID:Validation:ModelMissing', ...
    'Validation model not found: %s', modelFile);

plant = vafid_validation_model_data('plant');
criteria = vafid_validation_model_data('criteria');
controller = vafid_validation_model_data('controller');
scenarios = localScenarioDefinitions(controller.speedSlewRate_rpmps);
if strlength(string(scenarioFilter)) > 0
    keep = contains(string({scenarios.Name}), string(scenarioFilter), ...
        'IgnoreCase', true);
    scenarios = scenarios(keep);
end
assert(~isempty(scenarios), 'VAFID:Validation:NoScenarios', ...
    'No validation scenario matches "%s".', string(scenarioFilter));

simulationInputs(1, numel(scenarios)) = Simulink.SimulationInput(modelName);
for scenarioIndex = 1:numel(scenarios)
    scenario = scenarios(scenarioIndex);
    inputDataset = localBuildExternalInput(scenario, plant.sampleTime_s);
    simulationInputs(scenarioIndex) = Simulink.SimulationInput(modelName);
    simulationInputs(scenarioIndex) = ...
        simulationInputs(scenarioIndex).setExternalInput(inputDataset);
    simulationInputs(scenarioIndex) = simulationInputs(scenarioIndex) ...
        .setModelParameter( ...
        'StopTime', sprintf('%.9g', scenario.StopTime_s), ...
        'SimulationMode', 'normal', ...
        'SaveOutput', 'on', ...
        'OutputSaveName', 'yout', ...
        'SaveFormat', 'Dataset');
end

simulationOutputs = sim(simulationInputs, 'UseFastRestart', 'on');
rows = repmat(localEmptyResultRow(), numel(scenarios), 1);
for scenarioIndex = 1:numel(scenarios)
    rows(scenarioIndex) = localAssessScenario(simulationOutputs(scenarioIndex), ...
        scenarios(scenarioIndex), plant, controller, criteria);
end
results = struct2table(rows);
disp(results(:, {'Scenario', 'FinalSpeed_rpm', 'FinalLoad_Nm', ...
    'FirstValid_s', 'RsEst_Ohm', 'RsErr_pct', 'LdErr_pct', 'LqErr_pct', ...
    'FluxErr_pct', 'SpeedRMSE_rpm', 'CurrentRMSE_A', ...
    'ProbeRatioD', 'ProbeRatioQ', 'Pass'}));

if throwOnFailure && any(~results.Pass)
    failedNames = strjoin(results.Scenario(~results.Pass), ', ');
    error('VAFID:Validation:ClosedLoopFailure', ...
        'Closed-loop VAFID validation failed: %s', failedNames);
end
end

function scenarios = localScenarioDefinitions(speedSlewRate_rpmps)
speeds_rpm = [500, 3000, 7000];
loads_Nm = [0, 3, 7];
scenarios = repmat(localEmptyScenario(), 0, 1);
for speedIndex = 1:numel(speeds_rpm)
    for loadIndex = 1:numel(loads_Nm)
        speed_rpm = speeds_rpm(speedIndex);
        load_Nm = loads_Nm(loadIndex);
        transitionEnd_s = abs(speed_rpm) / speedSlewRate_rpmps;
        stopTime_s = transitionEnd_s + 3.0;
        name = sprintf('Steady_%drpm_%dNm', speed_rpm, load_Nm);
        scenarios(end + 1) = localMakeScenario(name, 0, speed_rpm, ... %#ok<AGROW>
            0, load_Nm, transitionEnd_s, stopTime_s);
    end
end

transitionEnd_s = 3000 / speedSlewRate_rpmps;
scenarios(end + 1) = localMakeScenario('Steady_Neg3000rpm_Neg3Nm', ...
    0, -3000, 0, -3, transitionEnd_s, transitionEnd_s + 3.0);

scenarios(end + 1) = localMakeScenario('VariableLoad_1to7to2Nm', ...
    0, 3000, [0, 1.8, 3.3], [1, 7, 2], 3.3, 6.3);

lastSpeedTransitionEnd_s = 3.4 + abs(5000 - 2000) / speedSlewRate_rpmps;
scenarios(end + 1) = localMakeScenario('VariableSpeed_1000to5000to2000rpm', ...
    [0, 1.5, 3.4], [1000, 5000, 2000], 0, 3, ...
    lastSpeedTransitionEnd_s, lastSpeedTransitionEnd_s + 3.0);

reversalEnd_s = 2.0 + abs(3000 - (-3000)) / speedSlewRate_rpmps;
scenarios(end + 1) = localMakeScenario('Reversal_3000toNeg3000rpm', ...
    [0, 2.0], [3000, -3000], [0, 2.0], [3, -3], ...
    reversalEnd_s, reversalEnd_s + 3.0);
end

function scenario = localMakeScenario(name, speedTimes_s, speedValues_rpm, ...
    loadTimes_s, loadValues_Nm, transitionEnd_s, stopTime_s)
scenario = localEmptyScenario();
scenario.Name = name;
scenario.SpeedTimes_s = speedTimes_s;
scenario.SpeedValues_rpm = speedValues_rpm;
scenario.LoadTimes_s = loadTimes_s;
scenario.LoadValues_Nm = loadValues_Nm;
scenario.TransitionEnd_s = transitionEnd_s;
scenario.StopTime_s = stopTime_s;
end

function scenario = localEmptyScenario()
scenario = struct( ...
    'Name', '', ...
    'SpeedTimes_s', 0, ...
    'SpeedValues_rpm', 0, ...
    'LoadTimes_s', 0, ...
    'LoadValues_Nm', 0, ...
    'TransitionEnd_s', 0, ...
    'StopTime_s', 0);
end

function inputDataset = localBuildExternalInput(scenario, sampleTime_s)
time_s = (0:sampleTime_s:scenario.StopTime_s).';
speedCommand_rpm = localPiecewiseConstant(time_s, ...
    scenario.SpeedTimes_s, scenario.SpeedValues_rpm);
loadTorque_Nm = localPiecewiseConstant(time_s, ...
    scenario.LoadTimes_s, scenario.LoadValues_Nm);

speedSeries = timeseries(speedCommand_rpm, time_s);
speedSeries.Name = 'SpeedCmd_rpm';
loadSeries = timeseries(loadTorque_Nm, time_s);
loadSeries.Name = 'LoadTorque_Nm';
inputDataset = Simulink.SimulationData.Dataset;
inputDataset = inputDataset.addElement(speedSeries, 'SpeedCmd_rpm');
inputDataset = inputDataset.addElement(loadSeries, 'LoadTorque_Nm');
end

function values = localPiecewiseConstant(time_s, breakTimes_s, breakValues)
values = repmat(double(breakValues(1)), size(time_s));
for breakIndex = 2:numel(breakTimes_s)
    values(time_s >= breakTimes_s(breakIndex)) = ...
        double(breakValues(breakIndex));
end
end

function row = localAssessScenario(simulationOutput, scenario, plant, ...
    controller, criteria)
if strlength(string(simulationOutput.ErrorMessage)) > 0
    error('VAFID:Validation:SimulationError', '%s: %s', ...
        scenario.Name, simulationOutput.ErrorMessage);
end
dataset = simulationOutput.get('yout');

[time_s, speedReference_rpm] = localOutput(dataset, 'SpeedRef_rpm');
[~, speedActual_rpm] = localOutput(dataset, 'SpeedActual_rpm');
[~, loadTorque_Nm] = localOutput(dataset, 'LoadTorqueEcho_Nm');
[~, id_A] = localOutput(dataset, 'Id_A');
[~, iq_A] = localOutput(dataset, 'Iq_A');
[~, currentError_A] = localOutput(dataset, 'CurrentError_A');
[~, voltageUtilization_PU] = localOutput(dataset, 'VoltageUtilization_PU');
[~, rs_Ohm] = localOutput(dataset, 'Rs_Ohm');
[~, ld_H] = localOutput(dataset, 'Ld_H');
[~, lq_H] = localOutput(dataset, 'Lq_H');
[~, fluxPM_Wb] = localOutput(dataset, 'FluxPM_Wb');
[~, estimateValid] = localOutput(dataset, 'EstimateValid');
[~, conditionNumber] = localOutput(dataset, 'ConditionNumber');
[~, relativeResidual] = localOutput(dataset, 'RelativeResidual');
[~, acceptedWindows] = localOutput(dataset, 'AcceptedWindows');
[~, probeD_PU] = localOutput(dataset, 'ProbeD_PU');
[~, probeQ_PU] = localOutput(dataset, 'ProbeQ_PU');

finalStart_s = max(scenario.TransitionEnd_s + criteria.postEventGuard_s, ...
    scenario.StopTime_s - criteria.finalAssessmentWindow_s);
finalMask = time_s >= finalStart_s;
postTransitionMask = time_s >= scenario.TransitionEnd_s;
validFinalMask = finalMask & estimateValid > 0.5;

assert(any(finalMask), 'VAFID:Validation:EmptyAssessmentWindow', ...
    '%s has no final assessment samples.', scenario.Name);

row = localEmptyResultRow();
row.Scenario = string(scenario.Name);
row.FinalSpeed_rpm = speedReference_rpm(end);
row.FinalLoad_Nm = loadTorque_Nm(end);
row.FirstValid_s = localFirstTime(time_s, estimateValid > 0.5);
row.Requalified_s = localFirstTime(time_s, ...
    time_s >= scenario.TransitionEnd_s & estimateValid > 0.5);
row.ValidFraction = mean(double(estimateValid(finalMask) > 0.5));
row.AcceptedWindows = double(min(acceptedWindows(finalMask)));

row.RsEst_Ohm = median(rs_Ohm(finalMask));
row.LdEst_H = median(ld_H(finalMask));
row.LqEst_H = median(lq_H(finalMask));
row.FluxEst_Wb = median(fluxPM_Wb(finalMask));
row.RsErr_pct = localRelativeErrorPercent(row.RsEst_Ohm, plant.rs_Ohm);
row.LdErr_pct = localRelativeErrorPercent(row.LdEst_H, plant.ld_H);
row.LqErr_pct = localRelativeErrorPercent(row.LqEst_H, plant.lq_H);
row.FluxErr_pct = localRelativeErrorPercent(row.FluxEst_Wb, plant.fluxPM_Wb);
row.RsDrift_pct = localPeakToPeakPercent(rs_Ohm(finalMask), plant.rs_Ohm);
row.LdDrift_pct = localPeakToPeakPercent(ld_H(finalMask), plant.ld_H);
row.LqDrift_pct = localPeakToPeakPercent(lq_H(finalMask), plant.lq_H);

speedError_rpm = speedReference_rpm(finalMask) - speedActual_rpm(finalMask);
row.SpeedMAE_rpm = mean(abs(speedError_rpm));
row.SpeedRMSE_rpm = sqrt(mean(speedError_rpm .^ 2));
row.SpeedRecovery_s = localRecoveryTime(time_s, ...
    speedReference_rpm, speedActual_rpm, scenario.TransitionEnd_s, ...
    criteria.speedRecoveryBand_pct, criteria.speedRecoveryHold_s);
row.CurrentRMSE_A = sqrt(mean(currentError_A(finalMask) .^ 2));
row.CurrentPeak_A = max(hypot(id_A(postTransitionMask), ...
    iq_A(postTransitionMask)));
row.VoltageUtilMax_PU = max(voltageUtilization_PU(finalMask));

if any(validFinalMask)
    row.RelativeResidual = max(relativeResidual(validFinalMask));
    row.ConditionNumber = max(conditionNumber(validFinalMask));
else
    row.RelativeResidual = Inf;
    row.ConditionNumber = Inf;
end

[row.ProbeRatioD, row.ProbePhaseD_deg] = localProbeTransfer( ...
    time_s(finalMask), id_A(finalMask), ...
    controller.currentBase_A * probeD_PU(finalMask), 150.0);
[row.ProbeRatioQ, row.ProbePhaseQ_deg] = localProbeTransfer( ...
    time_s(finalMask), iq_A(finalMask), ...
    controller.currentBase_A * probeQ_PU(finalMask), 220.0);

row.AllFinite = all(isfinite([speedActual_rpm; id_A; iq_A; rs_Ohm; ...
    ld_H; lq_H; fluxPM_Wb])) && isfinite(row.RelativeResidual) && ...
    isfinite(row.ConditionNumber);
speedScale_rpm = max(abs(row.FinalSpeed_rpm), 500.0);
row.Pass = row.AllFinite && ...
    row.FirstValid_s >= 1.25 - 2.0 * plant.sampleTime_s && ...
    row.ValidFraction >= criteria.validFractionMinimum && ...
    row.AcceptedWindows >= 10.0 && ...
    row.RsErr_pct <= criteria.rsErrorLimit_pct && ...
    row.LdErr_pct <= criteria.ldErrorLimit_pct && ...
    row.LqErr_pct <= criteria.lqErrorLimit_pct && ...
    row.FluxErr_pct <= criteria.fluxErrorLimit_pct && ...
    row.RsDrift_pct <= 2.0 && row.LdDrift_pct <= 3.0 && ...
    row.LqDrift_pct <= 3.0 && ...
    row.SpeedRMSE_rpm <= criteria.speedRmsErrorLimit_pct * ...
        speedScale_rpm / 100.0 && ...
    row.SpeedRecovery_s <= criteria.speedRecoveryLimit_s && ...
    row.CurrentRMSE_A <= criteria.currentRmsErrorLimit_A && ...
    row.CurrentPeak_A < 0.85 * controller.currentBase_A && ...
    row.VoltageUtilMax_PU <= criteria.voltageUtilizationLimit_PU && ...
    row.RelativeResidual <= criteria.relativeResidualLimit && ...
    row.ConditionNumber <= criteria.conditionNumberLimit && ...
    row.ProbeRatioD >= criteria.probeAmplitudeRatioMinimum && ...
    row.ProbeRatioD <= criteria.probeAmplitudeRatioMaximum && ...
    row.ProbeRatioQ >= criteria.probeAmplitudeRatioMinimum && ...
    row.ProbeRatioQ <= criteria.probeAmplitudeRatioMaximum && ...
    row.ProbePhaseD_deg <= criteria.probePhaseErrorLimit_deg && ...
    row.ProbePhaseQ_deg <= criteria.probePhaseErrorLimit_deg;
end

function row = localEmptyResultRow()
row = struct( ...
    'Scenario', "", ...
    'FinalSpeed_rpm', NaN, ...
    'FinalLoad_Nm', NaN, ...
    'FirstValid_s', Inf, ...
    'Requalified_s', Inf, ...
    'ValidFraction', 0.0, ...
    'AcceptedWindows', 0.0, ...
    'RsEst_Ohm', NaN, ...
    'LdEst_H', NaN, ...
    'LqEst_H', NaN, ...
    'FluxEst_Wb', NaN, ...
    'RsErr_pct', Inf, ...
    'LdErr_pct', Inf, ...
    'LqErr_pct', Inf, ...
    'FluxErr_pct', Inf, ...
    'RsDrift_pct', Inf, ...
    'LdDrift_pct', Inf, ...
    'LqDrift_pct', Inf, ...
    'RelativeResidual', Inf, ...
    'ConditionNumber', Inf, ...
    'SpeedMAE_rpm', Inf, ...
    'SpeedRMSE_rpm', Inf, ...
    'SpeedRecovery_s', Inf, ...
    'CurrentRMSE_A', Inf, ...
    'CurrentPeak_A', Inf, ...
    'VoltageUtilMax_PU', Inf, ...
    'ProbeRatioD', Inf, ...
    'ProbeRatioQ', Inf, ...
    'ProbePhaseD_deg', Inf, ...
    'ProbePhaseQ_deg', Inf, ...
    'AllFinite', false, ...
    'Pass', false);
end

function [time_s, values] = localOutput(dataset, portName)
for elementIndex = 1:dataset.numElements
    element = dataset{elementIndex};
    blockPath = string(element.BlockPath.getBlock(1));
    if endsWith(blockPath, "/" + string(portName))
        time_s = double(element.Values.Time(:));
        values = double(element.Values.Data(:));
        return;
    end
end
error('VAFID:Validation:OutputMissing', ...
    'Model output "%s" was not found.', portName);
end

function error_pct = localRelativeErrorPercent(value, truth)
error_pct = 100.0 * abs(value - truth) / abs(truth);
end

function drift_pct = localPeakToPeakPercent(values, truth)
drift_pct = 100.0 * (max(values) - min(values)) / abs(truth);
end

function firstTime_s = localFirstTime(time_s, mask)
firstIndex = find(mask, 1, 'first');
if isempty(firstIndex)
    firstTime_s = Inf;
else
    firstTime_s = time_s(firstIndex);
end
end

function recoveryTime_s = localRecoveryTime(time_s, reference_rpm, ...
    actual_rpm, transitionEnd_s, band_pct, holdTime_s)
sampleTime_s = median(diff(time_s));
holdSamples = max(1, round(holdTime_s / sampleTime_s));
band_rpm = band_pct / 100.0 * max(abs(reference_rpm(end)), 500.0);
insideBand = abs(reference_rpm - actual_rpm) <= band_rpm;
eligible = find(time_s >= transitionEnd_s);
recoveryTime_s = Inf;
for candidate = eligible(:).'
    lastIndex = candidate + holdSamples - 1;
    if lastIndex <= numel(insideBand) && all(insideBand(candidate:lastIndex))
        recoveryTime_s = time_s(candidate) - transitionEnd_s;
        return;
    end
end
end

function [amplitudeRatio, phaseError_deg] = localProbeTransfer( ...
    time_s, response_A, command_A, frequency_Hz)
responseAC_A = response_A - mean(response_A);
commandAC_A = command_A - mean(command_A);
kernel = exp(-1i * 2.0 * pi * frequency_Hz * time_s);
responsePhasor = sum(responseAC_A .* kernel);
commandPhasor = sum(commandAC_A .* kernel);
if abs(commandPhasor) <= eps
    amplitudeRatio = Inf;
    phaseError_deg = Inf;
else
    amplitudeRatio = abs(responsePhasor) / abs(commandPhasor);
    phaseError_deg = abs(rad2deg(angle(responsePhasor / commandPhasor)));
end
end
