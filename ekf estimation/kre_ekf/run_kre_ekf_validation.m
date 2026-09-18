function results = run_kre_ekf_validation()
%RUN_KRE_EKF_VALIDATION Validate the serial KRE active-flux and FluxEKF path.
%
% KRE estimates active flux from V_alpha, V_beta, I_alpha, and I_beta.
% FluxEKF normalizes that vector and is the sole source of final position
% and speed. This harness does not use the legacy KRE PLL output.

rootDir = fileparts(mfilename('fullpath'));
resolverDir = fullfile(fileparts(rootDir), 'resolver_ekf');
addpath(rootDir, resolverDir);

assert(exist('kre_ekf_kre_step', 'file') ~= 0, ...
    'kre_ekf:MissingKreStep', 'The local KRE active-flux step is missing.');
assert(exist('resolver_ekf_step', 'file') ~= 0, ...
    'kre_ekf:MissingResolverEkf', 'The resolver EKF step is missing.');

config = kre_ekf_default_config();
paperGains = resolver_ekf_design(0.1, 5000);
assert(isequal(size(paperGains), [3, 1]) && ...
    max(abs(paperGains - [0.2243154; 0.2445381; 0.0126402])) < 5e-4, ...
    'kre_ekf:PaperGainMismatch', 'Paper gain regression failed.');
assert(isequal(size(config.resolverEkfGains), [3, 1]) && ...
    max(abs(config.resolverEkfGains - [0.2868531; 0.1894056; 0.0122495])) < 5e-4, ...
    'kre_ekf:ProjectGainMismatch', 'Project-rate gain regression failed.');

signals = kre_ekf_make_validation_signals(config);
modelFile = build_kre_ekf_validation_model();
assert(isfile(modelFile), 'kre_ekf:MissingModel', ...
    'KRE-to-FluxEKF validation model was not built.');

outputs = local_run_model(modelFile, signals, config.resolverEkfConfig);
local_assert_outputs(outputs, signals, config.activeFluxMinimumWb);

metrics = local_calculate_metrics(signals, outputs, config.kreWarmupSec);
assert(metrics.validSampleCount >= config.minimumKreValidSamples, ...
    'kre_ekf:FluxInitialization', 'Not enough valid KRE-to-FluxEKF samples.');
assert(metrics.validRatio >= config.minimumKreValidRatio, ...
    'kre_ekf:FluxValidity', 'KRE active flux valid ratio is below the target.');
assert(metrics.circularAngleRmsRad < 0.02, ...
    'kre_ekf:CascadeAngleAccuracy', ...
    'KRE-to-FluxEKF angle RMS exceeds 0.02 rad.');
assert(metrics.speedRmsePU < 0.02, ...
    'kre_ekf:CascadeSpeedAccuracy', ...
    'KRE-to-FluxEKF speed RMSE exceeds 0.02 PU.');

plotPath = local_save_plot(rootDir, signals, outputs, metrics);
results = struct( ...
    'paperGains', paperGains, ...
    'projectGains', config.resolverEkfGains, ...
    'metrics', metrics, ...
    'plotPath', plotPath, ...
    'sampleCount', numel(signals.time));

fprintf(['KRE-to-FluxEKF validation passed: valid %.1f%%, flux %.5f Wb, ', ...
    'angle RMS %.5f rad, speed RMSE %.5f PU.\n'], ...
    100 * metrics.validRatio, metrics.meanActiveFluxMagnitudeWb, ...
    metrics.circularAngleRmsRad, metrics.speedRmsePU);
end

function outputs = local_run_model(modelFile, signals, resolverEkfConfig)
[modelDir, modelName] = fileparts(modelFile);
addpath(modelDir);
wasLoaded = bdIsLoaded(modelName);
if ~wasLoaded
    load_system(modelFile);
end
cleanup = onCleanup(@() local_close_model(modelName, wasLoaded)); %#ok<NASGU>

assert(strcmp(get_param(modelName, 'SolverType'), 'Fixed-step'), ...
    'kre_ekf:VariableStepModel', 'Model must use a fixed-step solver.');
assert(abs(str2double(get_param(modelName, 'FixedStep')) - resolverEkfConfig.Ts) < 1e-12, ...
    'kre_ekf:SampleTimeMismatch', 'Model sample time must equal 50 us.');

time = signals.time;
simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('truePositionPUInput', ...
    timeseries(signals.positionPUTrue, time));
simIn = simIn.setVariable('trueSpeedPUInput', ...
    timeseries(signals.speedPUTrue, time));
simIn = simIn.setVariable('kreEnableInput', timeseries(signals.kreEnable, time));
simIn = simIn.setVariable('kreViFbInput', timeseries(signals.kreViFbPU, time));
simIn = simIn.setVariable('kreParamsInput', timeseries(signals.kreParams, time));
simIn = simIn.setVariable('kreResetInput', timeseries(signals.kreReset, time));
simIn = simIn.setVariable('krePllParamsInput', ...
    timeseries(signals.krePllParams, time));
simIn = simIn.setVariable('resolver_ekf_config', resolverEkfConfig);
simIn = simIn.setModelParameter('StopTime', sprintf('%.17g', time(end)));
simOut = sim(simIn);

outputs = struct();
outputs.positionPUTrue = local_get_signal(simOut, 'positionPU_true', time);
outputs.speedPUTrue = local_get_signal(simOut, 'speedPU_true', time);
outputs.activeFluxAlphaWb = local_get_signal(simOut, ...
    'kre_active_flux_alpha_wb', time);
outputs.activeFluxBetaWb = local_get_signal(simOut, ...
    'kre_active_flux_beta_wb', time);
outputs.kreStatus = local_get_signal(simOut, 'kre_status', time);
outputs.fluxCos = local_get_signal(simOut, 'ekf_flux_cos', time);
outputs.fluxSin = local_get_signal(simOut, 'ekf_flux_sin', time);
outputs.fluxValid = local_get_signal(simOut, 'ekf_flux_valid', time);
outputs.ekfPositionPU = local_get_signal(simOut, 'ekf_positionPU', time);
outputs.ekfSpeedPU = local_get_signal(simOut, 'ekf_speedPU', time);
end

function local_close_model(modelName, wasLoaded)
if ~wasLoaded && bdIsLoaded(modelName)
    close_system(modelName, 0);
end
end

function data = local_get_signal(simOut, name, expectedTime)
values = simOut.get(name);
time = values.Time(:);
data = double(squeeze(values.Data));
data = data(:);
assert(numel(time) == numel(data), 'kre_ekf:LogSize', ...
    'Logged signal %s has inconsistent dimensions.', name);

if numel(time) == numel(expectedTime) && max(abs(time - expectedTime)) < 1e-12
    return
end

[time, index] = unique(time, 'stable');
data = data(index);
assert(numel(time) > 1 && expectedTime(1) >= time(1) && ...
    expectedTime(end) <= time(end), 'kre_ekf:LogCoverage', ...
    'Logged signal %s does not cover the test interval.', name);
data = interp1(time, data, expectedTime, 'linear');
end

function local_assert_outputs(outputs, signals, minimumFluxMagnitudeWb)
assert(max(abs(outputs.positionPUTrue - signals.positionPUTrue)) < 1e-12 && ...
    max(abs(outputs.speedPUTrue - signals.speedPUTrue)) < 1e-12, ...
    'kre_ekf:TruthLogMismatch', 'Truth logs differ from the applied inputs.');

allOutputs = [outputs.activeFluxAlphaWb; outputs.activeFluxBetaWb; ...
    outputs.kreStatus; outputs.fluxCos; outputs.fluxSin; outputs.fluxValid; ...
    outputs.ekfPositionPU; outputs.ekfSpeedPU];
assert(all(isfinite(allOutputs)), 'kre_ekf:NonFiniteOutput', ...
    'KRE-to-FluxEKF output contains NaN or Inf.');
assert(all(outputs.ekfPositionPU >= 0 & outputs.ekfPositionPU < 1), ...
    'kre_ekf:PositionRange', 'FluxEKF position must remain in [0, 1).');
assert(all(ismember(round(outputs.kreStatus), [0, 1, 2])) && ...
    all(ismember(round(outputs.fluxValid), [0, 1])), ...
    'kre_ekf:InvalidStatus', 'KRE or FluxEKF status is invalid.');

validMask = round(outputs.fluxValid) == 1;
assert(any(validMask), 'kre_ekf:NoFluxEkfSamples', ...
    'FluxEKF never received a valid KRE active-flux vector.');
fluxMagnitude = hypot(outputs.activeFluxAlphaWb(validMask), ...
    outputs.activeFluxBetaWb(validMask));
assert(all(fluxMagnitude >= minimumFluxMagnitudeWb), ...
    'kre_ekf:FluxMagnitude', 'FluxEKF accepted a sub-threshold active flux.');
unitCircleError = abs(outputs.fluxCos(validMask).^2 + ...
    outputs.fluxSin(validMask).^2 - 1);
assert(max(unitCircleError) < 1e-10, 'kre_ekf:FluxNormalization', ...
    'FluxEKF did not normalize the active-flux vector.');
end

function metrics = local_calculate_metrics(signals, outputs, warmupSec)
sampleCount = numel(signals.time);
warmMask = signals.time >= warmupSec;
validMask = warmMask & round(outputs.kreStatus) == 1 & ...
    round(outputs.fluxValid) == 1;

% resolver_ekf_step returns x(k+1) after consuming flux at k. Compare the
% returned state to the following truth sample, omitting the last point.
evaluationIndices = find(validMask);
evaluationIndices = evaluationIndices(evaluationIndices < sampleCount);
assert(~isempty(evaluationIndices), 'kre_ekf:NoEvaluationSamples', ...
    'No valid samples remain after warm-up and post-update alignment.');
truthIndices = evaluationIndices + 1;

angleError = atan2( ...
    sin(2 * pi * outputs.ekfPositionPU(evaluationIndices) - ...
        signals.thetaElectricalRadTrue(truthIndices)), ...
    cos(2 * pi * outputs.ekfPositionPU(evaluationIndices) - ...
        signals.thetaElectricalRadTrue(truthIndices)));
speedError = outputs.ekfSpeedPU(evaluationIndices) - ...
    signals.speedPUTrue(truthIndices);
activeFluxMagnitudeWb = hypot(outputs.activeFluxAlphaWb(evaluationIndices), ...
    outputs.activeFluxBetaWb(evaluationIndices));

metrics = struct();
metrics.validRatio = nnz(validMask) / nnz(warmMask);
metrics.validSampleCount = numel(evaluationIndices);
metrics.evaluationIndices = evaluationIndices;
metrics.circularAngleRmsRad = sqrt(mean(angleError .^ 2));
metrics.speedRmsePU = sqrt(mean(speedError .^ 2));
metrics.meanActiveFluxMagnitudeWb = mean(activeFluxMagnitudeWb);
metrics.minimumActiveFluxMagnitudeWb = min(activeFluxMagnitudeWb);
end

function plotPath = local_save_plot(rootDir, signals, outputs, metrics)
resultDir = fullfile(rootDir, 'results');
if ~isfolder(resultDir)
    mkdir(resultDir);
end
plotPath = fullfile(resultDir, 'kre_ekf_validation.png');

figureHandle = figure('Visible', 'off', 'Color', 'w', ...
    'Position', [100, 100, 1040, 840]);
cleanup = onCleanup(@() close(figureHandle)); %#ok<NASGU>
tiledlayout(3, 1, 'TileSpacing', 'compact', 'Padding', 'loose');

evaluationIndices = metrics.evaluationIndices;
truthIndices = evaluationIndices + 1;

nexttile;
plot(signals.time, local_wrap_to_pi(signals.thetaElectricalRadTrue), ...
    'k--', 'LineWidth', 1.0);
hold on;
plot(signals.time(truthIndices), ...
    local_wrap_to_pi(2 * pi * outputs.ekfPositionPU(evaluationIndices)), ...
    'Color', [0, 0.447, 0.741], 'LineWidth', 1.0);
grid on;
ylabel('Electrical angle (rad)');
legend({'True', 'KRE active flux -> EKF'}, 'Location', 'best');
title(sprintf('Post-update angle RMS: %.5f rad', ...
    metrics.circularAngleRmsRad));
local_style_axes(gca);

nexttile;
plot(signals.time, signals.speedPUTrue, 'k--', 'LineWidth', 1.0);
hold on;
plot(signals.time(truthIndices), outputs.ekfSpeedPU(evaluationIndices), ...
    'Color', [0, 0.447, 0.741], 'LineWidth', 1.0);
grid on;
ylabel('Mechanical speed (PU)');
legend({'True', 'KRE active flux -> EKF'}, 'Location', 'best');
title(sprintf('Post-update speed RMSE: %.5f PU', metrics.speedRmsePU));
local_style_axes(gca);

nexttile;
fluxMagnitude = hypot(outputs.activeFluxAlphaWb, outputs.activeFluxBetaWb);
plot(signals.time, fluxMagnitude, 'Color', [0.85, 0.325, 0.098], ...
    'LineWidth', 1.0);
hold on;
stairs(signals.time, 0.05 * round(outputs.fluxValid), ...
    'Color', [0.2, 0.2, 0.2], 'LineWidth', 1.0);
grid on;
xlabel('Time (s)');
ylabel('Active flux (Wb)');
legend({'KRE active flux magnitude', 'FluxEKF valid x 0.05'}, ...
    'Location', 'best');
title(sprintf('FluxEKF valid after warm-up: %.1f%%', ...
    100 * metrics.validRatio));
local_style_axes(gca);

exportgraphics(figureHandle, plotPath, 'Resolution', 150);
end

function local_style_axes(axisHandle)
set(axisHandle, 'Color', 'w', 'XColor', 'k', 'YColor', 'k', ...
    'GridColor', [0.7, 0.7, 0.7], 'GridAlpha', 0.8);
set(axisHandle.Title, 'Color', 'k');
legendHandle = axisHandle.Legend;
if ~isempty(legendHandle) && isvalid(legendHandle)
    set(legendHandle, 'Color', 'w', 'TextColor', 'k', ...
        'EdgeColor', [0.35, 0.35, 0.35]);
end
end

function angle = local_wrap_to_pi(angle)
angle = mod(angle + pi, 2 * pi) - pi;
end
