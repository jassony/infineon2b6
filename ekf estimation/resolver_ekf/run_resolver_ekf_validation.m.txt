function results = run_resolver_ekf_validation()
%RUN_RESOLVER_EKF_VALIDATION Deterministic paper and Simulink regression.
%
% This runner exercises the standalone resolver EKF only. It does not
% modify or depend on the FOC, FADO, or KRE observer integration paths.

rootDir = fileparts(mfilename('fullpath'));
addpath(rootDir);

assert(exist('resolver_ekf_default_config', 'file') == 2, ...
    'resolver_ekf_validation:MissingApi', ...
    'resolver_ekf_default_config.m is required on the MATLAB path.');
assert(exist('resolver_ekf_design', 'file') == 2, ...
    'resolver_ekf_validation:MissingApi', ...
    'resolver_ekf_design.m is required on the MATLAB path.');
assert(exist('resolver_ekf_step', 'file') == 2, ...
    'resolver_ekf_validation:MissingApi', ...
    'resolver_ekf_step.m is required on the MATLAB path.');

paperGains = resolver_ekf_design(0.1, 5000);
expectedPaperGains = [0.2243154; 0.2445381; 0.0126402];
assert(isequal(size(paperGains), [3, 1]), ...
    'resolver_ekf_validation:PaperGainShape', ...
    'The paper regression must return a 3-by-1 gain vector.');
assert(max(abs(paperGains - expectedPaperGains)) < 5e-4, ...
    'resolver_ekf_validation:PaperGainMismatch', ...
    'Paper gains do not match the T=0.1, lambda=5000 reference.');

cfg = resolver_ekf_default_config();
cfg.Ts = 50e-6;
cfg.baseMechanicalSpeedRPM = 10000;
cfg.polePairs = 4;
cfg.lambda = 5000;
cfg.beta = cfg.Ts * cfg.baseMechanicalSpeedRPM * cfg.polePairs * 2 * pi / 60;
cfg.gains = resolver_ekf_design(cfg);

expectedProjectGains = [0.2868531; 0.1894056; 0.0122495];
assert(isequal(size(cfg.gains), [3, 1]), ...
    'resolver_ekf_validation:ProjectGainShape', ...
    'The project-rate design must return a 3-by-1 gain vector.');
assert(max(abs(cfg.gains - expectedProjectGains)) < 5e-4, ...
    'resolver_ekf_validation:ProjectGainMismatch', ...
    'Project-rate gains do not match the 50-us KRE-compatible reference.');

[time, thetaERadTrue, positionPUTrue, speedPUTrue, cosMeas, sinMeas, reset] = ...
    make_validation_signals(cfg);
[directPositionPU, directSpeedPU, directInnovation] = ...
    run_direct_estimator(cosMeas, sinMeas, reset, cfg);

modelFile = fullfile(rootDir, 'resolver_ekf_validation.slx');
if ~isfile(modelFile)
    assert(exist('build_resolver_ekf_validation_model', 'file') == 2, ...
        'resolver_ekf_validation:MissingModelBuilder', ...
        'Expected model builder was not found: %s', rootDir);
    build_resolver_ekf_validation_model();
end
assert(isfile(modelFile), 'resolver_ekf_validation:MissingModel', ...
    'Expected validation model was not found: %s', modelFile);

[positionPUEst, speedPUEst, loggedSignals] = run_validation_model( ...
    modelFile, time, cosMeas, sinMeas, reset, positionPUTrue, speedPUTrue, cfg);

assert(all(isfinite(positionPUEst)) && all(isfinite(speedPUEst)), ...
    'resolver_ekf_validation:NonFiniteModelOutput', ...
    'The Simulink estimator output contains NaN or Inf.');
assert(all(isfinite(directPositionPU)) && all(isfinite(directSpeedPU)) && ...
    all(isfinite(directInnovation)), ...
    'resolver_ekf_validation:NonFiniteDirectOutput', ...
    'The direct estimator output contains NaN or Inf.');
assert(all(positionPUEst >= 0 & positionPUEst < 1), ...
    'resolver_ekf_validation:PositionRange', ...
    'The Simulink position output must remain in [0, 1).');

metrics = calculate_metrics(thetaERadTrue, speedPUTrue, cosMeas, sinMeas, ...
    positionPUEst, speedPUEst, cfg.beta);
directMetrics = calculate_metrics(thetaERadTrue, speedPUTrue, cosMeas, sinMeas, ...
    directPositionPU, directSpeedPU, cfg.beta);

assert(metrics.circularAngleRmsRad < 0.06, ...
    'resolver_ekf_validation:AngleAccuracy', ...
    'Circular electrical angle RMS %.6f rad exceeds 0.06 rad.', ...
    metrics.circularAngleRmsRad);
assert(metrics.speedRmsePU < 0.05, ...
    'resolver_ekf_validation:SpeedAccuracy', ...
    'Speed RMSE %.6f PU exceeds 0.05 PU.', metrics.speedRmsePU);
assert(metrics.speedRmsePU < metrics.atan2DifferenceSpeedRmsePU, ...
    'resolver_ekf_validation:BaselineComparison', ...
    'EKF speed RMSE must improve on the atan2-difference baseline.');

plotPath = save_validation_plot(rootDir, time, thetaERadTrue, ...
    positionPUEst, speedPUTrue, speedPUEst, metrics);

results = struct();
results.paperGains = paperGains;
results.projectGains = cfg.gains;
results.metrics = metrics;
results.directMetrics = directMetrics;
results.plotPath = plotPath;
results.sampleCount = numel(time);
results.loggedSignalNames = loggedSignals;

fprintf(['Resolver EKF validation passed: angle RMS %.5f rad, speed RMSE %.5f PU, ', ...
    'atan2 baseline %.5f PU.\n'], metrics.circularAngleRmsRad, ...
    metrics.speedRmsePU, metrics.atan2DifferenceSpeedRmsePU);
end

function [time, thetaERadTrue, positionPUTrue, speedPUTrue, cosMeas, sinMeas, reset] = ...
        make_validation_signals(cfg)
sampleCount = 1001;
sampleIndex = (0:sampleCount - 1).';
time = sampleIndex * cfg.Ts;
speedPUTrue = 1 - exp(-sampleIndex / 100);

thetaERadTrue = zeros(sampleCount, 1);
thetaERadTrue(2:end) = cumsum(cfg.beta * speedPUTrue(1:end - 1));
positionPUTrue = mod(thetaERadTrue / (2 * pi), 1);

rng(20260811, 'twister');
resolverNoise = 0.05 * randn(sampleCount, 2);
cosMeas = cos(thetaERadTrue) + resolverNoise(:, 1);
sinMeas = sin(thetaERadTrue) + resolverNoise(:, 2);
reset = zeros(sampleCount, 1);
reset(1) = 1;
end

function [positionPU, speedPU, innovation] = run_direct_estimator(cosMeas, sinMeas, reset, cfg)
sampleCount = numel(cosMeas);
positionPU = zeros(sampleCount, 1);
speedPU = zeros(sampleCount, 1);
innovation = zeros(sampleCount, 1);
state = zeros(3, 1);

for index = 1:sampleCount
    [state, positionPU(index), speedPU(index), innovation(index)] = ...
        resolver_ekf_step(state, cosMeas(index), sinMeas(index), reset(index), cfg);
end
end

function [positionPU, speedPU, signalNames] = run_validation_model( ...
        modelFile, time, cosMeas, sinMeas, reset, positionPUTrue, speedPUTrue, cfg)
[modelDir, modelName] = fileparts(modelFile);
addpath(modelDir);
wasLoaded = bdIsLoaded(modelName);
if ~wasLoaded
    load_system(modelFile);
end
modelCleanup = onCleanup(@() close_validation_model(modelName, wasLoaded)); %#ok<NASGU>

assert(strcmp(get_param(modelName, 'SolverType'), 'Fixed-step'), ...
    'resolver_ekf_validation:VariableStepModel', ...
    'The validation model must use a fixed-step solver.');
fixedStep = str2double(get_param(modelName, 'FixedStep'));
assert(isfinite(fixedStep) && abs(fixedStep - cfg.Ts) < 1e-12, ...
    'resolver_ekf_validation:UnexpectedFixedStep', ...
    'The validation model fixed step must equal 50 us.');

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('resolverCosInput', timeseries(cosMeas, time));
simIn = simIn.setVariable('resolverSinInput', timeseries(sinMeas, time));
simIn = simIn.setVariable('resolverResetInput', timeseries(reset, time));
simIn = simIn.setVariable('truePositionPUInput', timeseries(positionPUTrue, time));
simIn = simIn.setVariable('trueSpeedPUInput', timeseries(speedPUTrue, time));
simIn = simIn.setVariable('resolver_ekf_config', cfg);
simIn = simIn.setModelParameter('StopTime', sprintf('%.17g', time(end)));
simOut = sim(simIn);

requiredNames = {'resolver_cos', 'resolver_sin', 'resolver_reset', ...
    'positionPU_true', 'speedPU_true', 'positionPU_est', 'speedPU_est'};
for index = 1:numel(requiredNames)
    get_simulation_signal(simOut, requiredNames{index});
end

[cosTime, loggedCos] = get_simulation_signal(simOut, 'resolver_cos');
[sinTime, loggedSin] = get_simulation_signal(simOut, 'resolver_sin');
[resetTime, loggedReset] = get_simulation_signal(simOut, 'resolver_reset');
[truePositionTime, loggedTruePosition] = get_simulation_signal(simOut, 'positionPU_true');
[trueSpeedTime, loggedTrueSpeed] = get_simulation_signal(simOut, 'speedPU_true');
assert(max(abs(align_to_validation_time(cosTime, loggedCos, time, 'resolver_cos') - cosMeas)) < 1e-12, ...
    'resolver_ekf_validation:CosInputMismatch', 'Logged cosine input differs from the test vector.');
assert(max(abs(align_to_validation_time(sinTime, loggedSin, time, 'resolver_sin') - sinMeas)) < 1e-12, ...
    'resolver_ekf_validation:SinInputMismatch', 'Logged sine input differs from the test vector.');
assert(max(abs(align_to_validation_time(resetTime, loggedReset, time, 'resolver_reset') - reset)) < 1e-12, ...
    'resolver_ekf_validation:ResetInputMismatch', 'Logged reset input differs from the test vector.');
assert(max(abs(align_to_validation_time(truePositionTime, loggedTruePosition, time, 'positionPU_true') - positionPUTrue)) < 1e-12, ...
    'resolver_ekf_validation:PositionTruthMismatch', 'Logged position truth differs from the test vector.');
assert(max(abs(align_to_validation_time(trueSpeedTime, loggedTrueSpeed, time, 'speedPU_true') - speedPUTrue)) < 1e-12, ...
    'resolver_ekf_validation:SpeedTruthMismatch', 'Logged speed truth differs from the test vector.');

[positionTime, positionPU] = get_simulation_signal(simOut, 'positionPU_est');
[speedTime, speedPU] = get_simulation_signal(simOut, 'speedPU_est');
positionPU = align_to_validation_time(positionTime, positionPU, time, 'positionPU_est');
speedPU = align_to_validation_time(speedTime, speedPU, time, 'speedPU_est');
signalNames = requiredNames;
end

function close_validation_model(modelName, wasLoaded)
if ~wasLoaded && bdIsLoaded(modelName)
    close_system(modelName, 0);
end
end

function [signalTime, signalData] = get_simulation_signal(simOut, signalName)
try
    values = simOut.get(signalName);
    [signalTime, signalData] = unpack_signal_values(values, signalName);
    return;
catch
end

try
    logsout = simOut.get('logsout');
catch exception
    error('resolver_ekf_validation:MissingSimulationOutput', ...
        'Missing simulation output "%s" (%s).', signalName, exception.message);
end

try
    element = logsout.getElement(signalName);
catch exception
    error('resolver_ekf_validation:MissingLoggedSignal', ...
        'Missing logged signal "%s" (%s).', signalName, exception.message);
end

[signalTime, signalData] = unpack_signal_values(element.Values, signalName);
end

function [signalTime, signalData] = unpack_signal_values(values, signalName)
try
    signalTime = values.Time;
    signalData = values.Data;
catch exception
    error('resolver_ekf_validation:UnsupportedLoggedSignal', ...
        'Signal "%s" has an unsupported logged value (%s).', ...
        signalName, exception.message);
end

signalTime = signalTime(:);
signalData = squeeze(signalData);
signalData = signalData(:);
assert(numel(signalTime) == numel(signalData), ...
    'resolver_ekf_validation:LoggedSignalSize', ...
    'Logged signal "%s" has inconsistent time and data lengths.', signalName);
end

function alignedData = align_to_validation_time(signalTime, signalData, validationTime, signalName)
if numel(signalTime) == numel(validationTime) && ...
        max(abs(signalTime - validationTime)) < 1e-12
    alignedData = signalData;
    return;
end

[uniqueTime, uniqueIndex] = unique(signalTime, 'stable');
uniqueData = signalData(uniqueIndex);
assert(numel(uniqueTime) > 1, 'resolver_ekf_validation:InsufficientSamples', ...
    'Logged signal "%s" has too few samples to align.', signalName);
assert(validationTime(1) >= uniqueTime(1) && validationTime(end) <= uniqueTime(end), ...
    'resolver_ekf_validation:LoggedSignalCoverage', ...
    'Logged signal "%s" does not cover the validation time vector.', signalName);
alignedData = interp1(uniqueTime, uniqueData, validationTime, 'linear');
end

function metrics = calculate_metrics(thetaERadTrue, speedPUTrue, cosMeas, sinMeas, ...
        positionPUEst, speedPUEst, beta)
estimatedThetaERad = 2 * pi * positionPUEst;
angleError = atan2(sin(estimatedThetaERad - thetaERadTrue), ...
    cos(estimatedThetaERad - thetaERadTrue));

rawResolverAngle = unwrap(atan2(sinMeas, cosMeas));
atan2DifferenceSpeedPU = zeros(size(speedPUTrue));
atan2DifferenceSpeedPU(2:end) = diff(rawResolverAngle) / beta;

metrics = struct();
metrics.circularAngleRmsRad = sqrt(mean(angleError .^ 2));
metrics.speedRmsePU = sqrt(mean((speedPUEst - speedPUTrue) .^ 2));
metrics.atan2DifferenceSpeedRmsePU = sqrt(mean((atan2DifferenceSpeedPU - speedPUTrue) .^ 2));
end

function plotPath = save_validation_plot(rootDir, time, thetaERadTrue, ...
        positionPUEst, speedPUTrue, speedPUEst, metrics)
resultDir = fullfile(rootDir, 'results');
if ~isfolder(resultDir)
    mkdir(resultDir);
end
plotPath = fullfile(resultDir, 'resolver_ekf_validation.png');

figureHandle = figure('Visible', 'off', 'Color', 'w', 'Position', [100, 100, 960, 640]);
figureCleanup = onCleanup(@() close(figureHandle)); %#ok<NASGU>
tiledlayout(2, 1, 'TileSpacing', 'compact', 'Padding', 'compact');

nexttile;
plot(time, wrap_to_pi(thetaERadTrue), 'k--', 'LineWidth', 1.0);
hold on;
plot(time, wrap_to_pi(2 * pi * positionPUEst), 'b-', 'LineWidth', 1.0);
grid on;
ylabel('Electrical angle (rad)');
legend({'True', 'EKF'}, 'Location', 'best');
title(sprintf('Resolver EKF angle: circular RMS = %.4f rad', ...
    metrics.circularAngleRmsRad));

nexttile;
plot(time, speedPUTrue, 'k--', 'LineWidth', 1.0);
hold on;
plot(time, speedPUEst, 'b-', 'LineWidth', 1.0);
grid on;
xlabel('Time (s)');
ylabel('Mechanical speed (PU)');
legend({'True', 'EKF'}, 'Location', 'best');
title(sprintf('Speed RMSE = %.4f PU, atan2 baseline = %.4f PU', ...
    metrics.speedRmsePU, metrics.atan2DifferenceSpeedRmsePU));

exportgraphics(figureHandle, plotPath, 'Resolution', 150);
end

function wrappedAngle = wrap_to_pi(angleRad)
wrappedAngle = mod(angleRad + pi, 2 * pi) - pi;
end
