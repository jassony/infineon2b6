function report = run_fado_paper_reproduction(varargin)
%RUN_FADO_PAPER_REPRODUCTION Reproduce FADO mechanisms on the Teknic plant.
% The model remains on its nominal eight-second profile outside each
% SimulationInput run. This harness never saves the model or changes the
% default FADO test profile inputs in the base workspace.

parser = inputParser;
addParameter(parser, 'FastBandwidthHz', 150, @(x) isnumeric(x) && isscalar(x) && x > 0);
addParameter(parser, 'ScanFastBandwidth', true, @(x) islogical(x) && isscalar(x));
addParameter(parser, 'RunTableSweep', true, @(x) islogical(x) && isscalar(x));
addParameter(parser, 'RunParameterErrorCases', false, @(x) islogical(x) && isscalar(x));
addParameter(parser, 'ScenarioNames', string.empty(0, 1), ...
    @(x) ischar(x) || isstring(x) || iscellstr(x));
addParameter(parser, 'Fig10BiasStartTime', 3.5, ...
    @(x) isnumeric(x) && isscalar(x) && x >= 0);
addParameter(parser, 'Fig10LoadSignFollowsSpeed', true, ...
    @(x) islogical(x) && isscalar(x));
addParameter(parser, 'Fig10BiasAlphaV', 5e-3, ...
    @(x) isnumeric(x) && isscalar(x));
parse(parser, varargin{:});
options = parser.Results;

mdl = 'mcb_pmsm_foc_sensorless_f28379d';
open_system(mdl);

% Keep the model callback chain explicit so this script is self-contained.
mcb_pmsm_foc_sensorless_f28379d_datascript;
mcb_pmsm_foc_sensorless_f28379d_pi_recovery;
mcb_pmsm_foc_sensorless_f28379d_fado_init;
mcb_pmsm_foc_sensorless_f28379d_fado_tuning;
mcb_pmsm_foc_sensorless_f28379d_fado_testprofile_init;

assert(exist('fado_discrete_equations_v2', 'file') == 2, ...
    'run_fado_paper_reproduction:MissingV2Reference', ...
    'The required fado_discrete_equations_v2 reference is unavailable.');
referenceEquations = fado_discrete_equations_v2(fado, 0);
assert(isfield(referenceEquations, 'Observer') && isfield(referenceEquations, 'Flux') ...
    && isfield(referenceEquations, 'T2S'), ...
    'run_fado_paper_reproduction:InvalidV2Reference', ...
    'fado_discrete_equations_v2 did not return the expected v2 structure.');

ids = localModelIds(mdl);
scenarios = localBuildPaperScenarios(pmsm, options.Fig10BiasStartTime, ...
    options.Fig10LoadSignFollowsSpeed, options.Fig10BiasAlphaV);
requestedNames = string(options.ScenarioNames);
if ~isempty(requestedNames)
    selected = ismember(string({scenarios.Name}), requestedNames);
    assert(any(selected), 'run_fado_paper_reproduction:UnknownScenario', ...
        'No requested ScenarioNames matched the paper-reproduction scenarios.');
    scenarios = scenarios(selected);
end
results = repmat(localResultTemplate(), numel(scenarios), 1);

for index = 1:numel(scenarios)
    [results(index), ~] = localRunWithBandwidthScan( ...
        mdl, ids, scenarios(index), pmsm, fado, options);
    fprintf('%s: pass=%d, fastT2S=%g Hz, finite=%d, positionRMS=%0.3f deg\n', ...
        results(index).Name, results(index).Pass, results(index).FastBandwidthHz, ...
        results(index).Metrics.Finite, results(index).Metrics.PositionRMSDeg);
end

tableI = struct('Enabled', options.RunTableSweep, 'Rows', [], 'Runs', []);
if options.RunTableSweep
    tableI = localRunTableISweep(mdl, ids, pmsm, fado, options);
end

parameterCases = struct('Enabled', options.RunParameterErrorCases, 'Runs', []);
if options.RunParameterErrorCases
    parameterCases = localRunParameterErrorCases( ...
        mdl, ids, scenarios(1), pmsm, fado, options);
end

report = struct();
report.Model = mdl;
report.Options = options;
report.ReferenceEquations = referenceEquations;
report.Scenarios = results;
report.TableI = tableI;
report.ParameterErrorCases = parameterCases;
report.CreatedAt = datetime('now');
report.Summary = localSummary(results, tableI, parameterCases);
end

function ids = localModelIds(mdl)
ids = struct();
ids.Selector = Simulink.ID.getFullName([mdl ':7835']);
ids.IdAssist = Simulink.ID.getFullName([mdl ':9010']);
ids.FadoConfig = Simulink.ID.getFullName([mdl ':9012']);
end

function scenarios = localBuildPaperScenarios(pmsm, fig10BiasStartTime, fig10LoadSignFollowsSpeed, fig10BiasAlphaV)
ratedTorque = pmsm.T_rated;
load2 = 2/11.5*ratedTorque;
load3 = 3/11.5*ratedTorque;
load75 = 7.5/11.5*ratedTorque;
accel = 90/pmsm.N_base;

scenarios(1) = localScenario( ...
    'Fig10_flux_disturbance_reversal', 'fig10', 8.0, ...
    [0 0.25; 3.5 0.25; 5.0 0.10; 7.0 -0.10; 8.0 -0.10], ...
    localFig10LoadKnots(load2, fig10LoadSignFollowsSpeed), ...
    localBiasVectorKnots(fig10BiasStartTime, 8.1, fig10BiasAlphaV), ...
    false, [7.4 8.0]);

scenarios(2) = localScenario( ...
    'Fig11_kdf_disable_recovery', 'fig11', 11.5, ...
    [0 0.25; 3.5 0.25; 8.0 -0.20; 11.5 -0.20], ...
    [0 0; 3.5 -load3; 11.6 -load3], ...
    [0 0 0 0 0 0; 3.5 1 5e-3 0 1 0.5; 9.0 1 5e-3 0 1 0.0; ...
     10.0 1 5e-3 0 1 0.5; 11.6 1 5e-3 0 1 0.5], ...
    false, [10.5 11.5]);
scenarios(2).KdfWindows = [8.0 9.0; 9.0 10.0; 10.0 11.0];

scenarios(3) = localScenario( ...
    'Fig12_zero_speed_limiter', 'fig12', 7.5, ...
    [0 0.25; 3.5 0.25; 5.0 0.10; 6.0 0.00; 7.5 0.00], ...
    [0 0; 7.6 0], ...
    [0 0 0 0 0 0; 6.5 1 20e-3 0 0 0; 6.8 0 0 0 0 0; 7.6 0 0 0 0 0], ...
    true, [6.5 7.3]);
scenarios(3).BiasVoltage = 20e-3;

negativeEnd = 5.25;
zeroCross = negativeEnd + 0.1/accel;
positiveEnd = negativeEnd + 0.2/accel;
scenarios(4) = localScenario( ...
    'Fig13_high_load_reversal', 'fig13', 16.0, ...
    [0 0.25; 3.5 0.25; negativeEnd -0.10; positiveEnd 0.10; 16.0 0.10], ...
    [0 0; 3.5 -load75; zeroCross load75; 16.1 load75], ...
    [0 0 0 0 0 0; 16.1 0 0 0 0 0], ...
    false, [zeroCross - 0.6 zeroCross + 0.6]);
scenarios(4).ZeroCrossTime = zeroCross;

scenarios(5) = localScenario( ...
    'Fig14_three_zero_speed_holds', 'fig14', 8.5, ...
    [0 0.25; 3.5 0.25; 5.0 0.10; 5.3 0.00; 5.6 0.00; ...
     5.9 0.10; 6.2 0.00; 6.5 0.00; 6.8 0.10; 7.1 0.00; ...
     7.4 0.00; 7.7 0.10; 8.5 0.10], ...
    [0 0; 3.5 load75; 8.6 load75], ...
    [0 0 0 0 0 0; 8.6 0 0 0 0 0], ...
    true, [7.8 8.5]);
scenarios(5).HoldWindows = [5.3 5.6; 6.2 6.5; 7.1 7.4];
end

function knots = localBiasVectorKnots(startTime, stopTime, alphaBiasV)
if startTime <= 0
    knots = [0 1 alphaBiasV 0 0 0; stopTime 1 alphaBiasV 0 0 0];
else
    knots = [0 0 0 0 0 0; startTime 1 alphaBiasV 0 0 0; ...
        stopTime 1 alphaBiasV 0 0 0];
end
end

function knots = localFig10LoadKnots(loadMagnitude, signFollowsSpeed)
if signFollowsSpeed
    % Keep the test-machine torque opposing the commanded rotation.
    knots = [0 0; 3.5 loadMagnitude; 6.0 -loadMagnitude; 8.1 -loadMagnitude];
else
    knots = [0 0; 3.5 loadMagnitude; 8.1 loadMagnitude];
end
end

function scenario = localScenario(name, kind, stopTime, speedKnots, loadKnots, vectorKnots, enableIdAssist, evalWindow)
scenario = struct();
scenario.Name = name;
scenario.Kind = kind;
scenario.StopTime = stopTime;
scenario.SpeedKnots = speedKnots;
scenario.LoadKnots = loadKnots;
scenario.VectorKnots = vectorKnots;
scenario.EnableIdAssist = enableIdAssist;
scenario.EvalWindow = evalWindow;
scenario.RsScale = 1.0;
scenario.LqScale = 1.0;
scenario.KdfWindows = zeros(0, 2);
scenario.BiasVoltage = 0.0;
scenario.ZeroCrossTime = NaN;
scenario.HoldWindows = zeros(0, 2);
scenario.TargetSpeedPU = zeros(1, 0);
scenario.TargetWindows = zeros(0, 2);
end

function [result, scan] = localRunWithBandwidthScan(mdl, ids, scenario, pmsm, fado, options)
result = localRunScenario(mdl, ids, scenario, pmsm, fado, options.FastBandwidthHz);
scan = localBandwidthScanTemplate(result.FastBandwidthHz);

if ~options.ScanFastBandwidth || ~localShouldScanBandwidth(scenario, result)
    result.BandwidthScan = scan;
    return;
end

candidates = [100 125 150 175 200];
scan.Triggered = true;
scan.CandidatesHz = candidates;
scan.Pass = false(size(candidates));
scan.CandidateMetrics = repmat(localFailedMetrics(), numel(candidates), 1);
scan.CandidateErrors = strings(numel(candidates), 1);
candidateResults = repmat(result, numel(candidates), 1);
for index = 1:numel(candidates)
    if candidates(index) ~= result.FastBandwidthHz
        candidateResults(index) = localRunScenario( ...
            mdl, ids, scenario, pmsm, fado, candidates(index));
    end
    scan.Pass(index) = candidateResults(index).Pass;
    scan.CandidateMetrics(index) = candidateResults(index).Metrics;
    scan.CandidateErrors(index) = string(candidateResults(index).Error);
end

selected = find(scan.Pass, 1, 'first');
if ~isempty(selected)
    result = candidateResults(selected);
    scan.SelectedHz = candidates(selected);
end
result.BandwidthScan = scan;
end

function shouldScan = localShouldScanBandwidth(scenario, result)
if ~result.Metrics.Finite
    shouldScan = true;
    return;
end

switch scenario.Kind
    case {'fig12', 'fig14'}
        % These low-speed cases have explicit limiter/hold criteria instead.
        shouldScan = false;
    otherwise
        shouldScan = result.Metrics.Oscillatory;
end
end

function result = localRunScenario(mdl, ids, scenario, pmsm, fado, fastBandwidthHz)
result = localResultTemplate();
result.Name = scenario.Name;
result.Kind = scenario.Kind;
result.FastBandwidthHz = fastBandwidthHz;
result.Error = '';

try
    speedTs = localSpeedTimeseries(scenario.SpeedKnots, scenario.StopTime);
    loadTs = localZohTimeseries(scenario.LoadKnots, scenario.StopTime);
    vectorTs = localZohTimeseries(scenario.VectorKnots, scenario.StopTime);

    simIn = Simulink.SimulationInput(mdl);
    simIn = simIn.setModelParameter( ...
        'StopTime', num2str(scenario.StopTime, 16), ...
        'ReturnWorkspaceOutputs', 'on', ...
        'SignalLogging', 'on', ...
        'SignalLoggingName', 'logsout');
    simIn = simIn.setBlockParameter(ids.Selector, 'Value', 'uint32(3)');
    simIn = simIn.setBlockParameter(ids.IdAssist, 'Value', ...
        sprintf('single(%d)', scenario.EnableIdAssist));
    simIn = simIn.setBlockParameter(ids.FadoConfig, 'Value', ...
        localFadoConfigExpression(fastBandwidthHz));
    simIn = simIn.setVariable('FADO_TestProfileEnable', true);
    simIn = simIn.setVariable('FADO_TestSpeedCommandPU', speedTs);
    simIn = simIn.setVariable('FADO_TestLoadNm', loadTs);
    simIn = simIn.setVariable('FADO_TestVector', vectorTs);
    simIn = simIn.setVariable('fado_RsEstScale', single(scenario.RsScale));
    simIn = simIn.setVariable('fado_LqEstScale', single(scenario.LqScale));

    simOut = sim(simIn);
    result.LogNames = localLogNames(simOut.logsout);
    result.Metrics = localCollectMetrics(simOut.logsout, scenario, pmsm, fado);
    result.Mechanism = localMechanismChecks(simOut.logsout, scenario, fado, result.Metrics);
    if strcmp(scenario.Kind, 'tablei')
        [result.TablePointStable, result.TablePointMetrics] = ...
            localCollectTablePointStability(simOut.logsout, scenario, pmsm, fado);
    end
    result.Pass = localScenarioPass(scenario, result.Metrics, result.Mechanism);
catch exception
    result.Metrics = localFailedMetrics();
    result.Mechanism = localMechanismTemplate();
    result.Mechanism.Reason = 'simulation_error';
    result.Pass = false;
    result.Error = getReport(exception, 'basic', 'hyperlinks', 'off');
end
end

function pass = localScenarioPass(scenario, metrics, mechanism)
switch scenario.Kind
    case {'fig12', 'fig14'}
        % Zero-speed cases are accepted by their explicit limiter/hold criteria.
        pass = metrics.Finite && mechanism.Pass;
    otherwise
        pass = metrics.CommonStable && mechanism.Pass;
end
end

function expression = localFadoConfigExpression(fastBandwidthHz)
expression = sprintf([ ...
    'single([fado_Rs fado_RsEstScale fado_Lq fado_LqEstScale ' ...
    'fado_PolePairs fado_Ts fado_VBase fado_IBase fado_NBase ' ...
    'fado_FluxLimit fado_Kdf fado_LowSpeedHz fado_Kaf %0.16g ' ...
    'fado_T2SSlowBandwidthHz fado_Zeta fado_EnableVoltagePreprocess ' ...
    'fado_VAlphaPreprocess fado_VBetaPreprocess])'], fastBandwidthHz);
end

function ts = localSpeedTimeseries(knots, stopTime)
sampleTime = 0.1;
time = (0:sampleTime:stopTime + sampleTime)';
knots = localAppendHoldKnot(knots, stopTime + sampleTime);
data = interp1(knots(:, 1), knots(:, 2), time, 'linear', 'extrap');
ts = timeseries(single(data), time);
end

function ts = localZohTimeseries(knots, stopTime)
knots = localAppendHoldKnot(knots, stopTime + 5e-5);
ts = timeseries(single(knots(:, 2:end)), knots(:, 1));
end

function knots = localAppendHoldKnot(knots, finalTime)
if knots(end, 1) < finalTime
    knots(end + 1, :) = [finalTime knots(end, 2:end)];
end
end

function names = localLogNames(logsout)
names = strings(logsout.numElements, 1);
for index = 1:logsout.numElements
    names(index) = string(logsout{index}.Name);
end
end

function metrics = localCollectMetrics(logsout, scenario, pmsm, fado)
required = {'Pos_PU_est', 'speed_PU_est', 'Pos_PU_true', 'Iq_Ref', 'Iq_fb', ...
    'Enable', 'EnClosedLoop', 'FADO_lambda_alpha1', 'FADO_lambda_beta1', ...
    'FADO_Dhat_alpha', 'FADO_Dhat_beta', 'FADO_kdf_mode', 'FADO_kaf_mode', ...
    'FADO_flux_limited', 'FADO_kdf_effective', 'FADO_test_bias_active', ...
    'FADO_rotor_flux_mag', 'FADO_Id_Assist_Active'};
for index = 1:numel(required)
    localGetSignal(logsout, required{index});
end

[time, posEst] = localTimeData(logsout, 'Pos_PU_est');
[~, speedEst] = localTimeData(logsout, 'speed_PU_est');
[timeTrue, posTrueRaw] = localTimeData(logsout, 'Pos_PU_true');
posTrue = localSample(timeTrue, posTrueRaw, time);
angleError = atan2(sin(2*pi*(posEst - posTrue)), cos(2*pi*(posEst - posTrue)));

thetaTrue = unwrap(2*pi*posTrue);
speedTrue = gradient(thetaTrue, time)/(2*pi*pmsm.p)*(60/pmsm.N_base);

[timeIq, iqRef] = localTimeData(logsout, 'Iq_Ref');
[timeIqFb, iqFbRaw] = localTimeData(logsout, 'Iq_fb');
iqFb = localSample(timeIqFb, iqFbRaw, timeIq);

[timeClosed, enClosedRaw] = localTimeData(logsout, 'EnClosedLoop');
enClosed = localSample(timeClosed, enClosedRaw, time) ~= 0;
[timeEnable, enableRaw] = localTimeData(logsout, 'Enable');
enable = localSample(timeEnable, enableRaw, time) ~= 0;

window = time >= scenario.EvalWindow(1) & time <= scenario.EvalWindow(2);
if ~any(window)
    window = true(size(time));
end

finiteSignals = [posEst; speedEst; posTrue; speedTrue; iqRef; iqFb];
fadoNames = localLogNames(logsout);
for index = 1:numel(fadoNames)
    if startsWith(fadoNames(index), 'FADO_')
        [~, data] = localTimeData(logsout, char(fadoNames(index)));
        finiteSignals = [finiteSignals; data(:)]; %#ok<AGROW>
    end
end

metrics = struct();
metrics.Finite = all(isfinite(finiteSignals));
metrics.PositionRMSDeg = rad2deg(sqrt(mean(angleError(window).^2)));
metrics.PositionPeakRad = max(abs(angleError(window)));
metrics.SpeedErrorRMSPu = sqrt(mean((speedEst(window) - speedTrue(window)).^2));
metrics.SpeedOscillationPu = std(speedEst(window) - speedTrue(window));
metrics.IqTrackingRMSPu = sqrt(mean((localSample(timeIq, iqRef, time(window)) ...
    - localSample(timeIq, iqFb, time(window))).^2));
metrics.ClosedLoopRatio = mean(double(enClosed(window)));
metrics.EnableRatio = mean(double(enable(window)));
metrics.CommonStable = metrics.Finite && metrics.PositionRMSDeg <= 5.0 ...
    && metrics.ClosedLoopRatio >= 0.95 && metrics.EnableRatio >= 0.95 ...
    && metrics.SpeedOscillationPu <= 0.02;
metrics.Oscillatory = ~metrics.Finite || ...
    (metrics.ClosedLoopRatio >= 0.80 && metrics.SpeedOscillationPu > 0.05) || ...
    metrics.PositionPeakRad > 0.8;

[timeDhat, dhatAlpha] = localTimeData(logsout, 'FADO_Dhat_alpha');
[timeDhatBeta, dhatBetaRaw] = localTimeData(logsout, 'FADO_Dhat_beta');
metrics.DhatMagnitude = hypot(dhatAlpha, localSample(timeDhatBeta, dhatBetaRaw, timeDhat));
metrics.DhatTime = timeDhat;

[timeFlux, fluxMagnitude] = localTimeData(logsout, 'FADO_rotor_flux_mag');
metrics.RotorFluxTime = timeFlux;
metrics.RotorFluxMagnitude = fluxMagnitude;
metrics.FluxLimit = fado.FluxLimit;
end

function mechanism = localMechanismChecks(logsout, scenario, fado, metrics)
mechanism = localMechanismTemplate();

switch scenario.Kind
    case 'fig10'
        before = localWindowMean(metrics.DhatTime, metrics.DhatMagnitude, [3.0 3.45]);
        after = localWindowMean(metrics.DhatTime, metrics.DhatMagnitude, [7.4 8.0]);
        mechanism.Details.DhatBefore = before;
        mechanism.Details.DhatAfter = after;
        mechanism.Details.DhatExcited = after > before + 1e-7;
        mechanism.Pass = metrics.Finite && mechanism.Details.DhatExcited;
        mechanism.Reason = 'disturbance_response';

    case 'fig11'
        [timeA, lambdaA] = localTimeData(logsout, 'FADO_lambda_alpha1');
        [timeB, lambdaBRaw] = localTimeData(logsout, 'FADO_lambda_beta1');
        lambdaMagnitude = hypot(lambdaA, localSample(timeB, lambdaBRaw, timeA));
        drifts = zeros(3, 1);
        for index = 1:3
            drifts(index) = localWindowDrift(timeA, lambdaMagnitude, scenario.KdfWindows(index, :));
        end
        mechanism.Details.Lambda1Drift = drifts;
        mechanism.Details.KdfOffIncreasesDrift = drifts(2) > 1.1*max(drifts(1), 1e-8);
        recoveryMask = timeA >= 10.5 & timeA <= 11.5;
        mechanism.Details.RecoveryFluxFinite = all(isfinite(lambdaMagnitude(recoveryMask)));
        mechanism.Pass = metrics.Finite && mechanism.Details.KdfOffIncreasesDrift ...
            && mechanism.Details.RecoveryFluxFinite && metrics.CommonStable;
        mechanism.Reason = 'kdf_disable_and_recovery';

    case 'fig12'
        [timeKaf, kafMode] = localTimeData(logsout, 'FADO_kaf_mode');
        [timeLimit, limitFlag] = localTimeData(logsout, 'FADO_flux_limited');
        [timeAssist, idAssist] = localTimeData(logsout, 'FADO_Id_Assist_Active');
        [timeFlux, fluxMagnitude] = localTimeData(logsout, 'FADO_rotor_flux_mag');
        window = scenario.EvalWindow;
        mechanism.Details.KafModeActive = any(localWindowData(timeKaf, kafMode, window) ~= 0);
        mechanism.Details.IdAssistActive = any(localWindowData(timeAssist, idAssist, window) ~= 0);
        mechanism.Details.FluxLimited = any(localWindowData(timeLimit, limitFlag, window) ~= 0);
        fluxBound = fado.FluxLimit + abs(scenario.BiasVoltage)/fado.Kaf + 2e-5;
        mechanism.Details.FluxBoundWb = fluxBound;
        mechanism.Details.MaxRotorFluxWb = max(localWindowData(timeFlux, fluxMagnitude, window));
        mechanism.Details.FluxBounded = mechanism.Details.MaxRotorFluxWb <= fluxBound;
        mechanism.Pass = metrics.Finite && mechanism.Details.KafModeActive ...
            && mechanism.Details.IdAssistActive && mechanism.Details.FluxLimited ...
            && mechanism.Details.FluxBounded;
        mechanism.Reason = 'low_speed_limiter';

    case 'fig13'
        [time, posEst] = localTimeData(logsout, 'Pos_PU_est');
        [timeTrue, posTrueRaw] = localTimeData(logsout, 'Pos_PU_true');
        posTrue = localSample(timeTrue, posTrueRaw, time);
        error = atan2(sin(2*pi*(posEst - posTrue)), cos(2*pi*(posEst - posTrue)));
        crossError = localWindowData(time, error, scenario.EvalWindow);
        mechanism.Details.ZeroCrossPeakRad = max(abs(crossError));
        mechanism.Details.ClosedLoopAcrossCrossing = metrics.ClosedLoopRatio >= 0.95;
        mechanism.Pass = metrics.Finite && mechanism.Details.ClosedLoopAcrossCrossing ...
            && mechanism.Details.ZeroCrossPeakRad <= 0.25;
        mechanism.Reason = 'high_load_reversal';

    case 'fig14'
        [timeClosed, enClosed] = localTimeData(logsout, 'EnClosedLoop');
        [timeEnable, enable] = localTimeData(logsout, 'Enable');
        [timeKaf, kafMode] = localTimeData(logsout, 'FADO_kaf_mode');
        [timeAssist, idAssist] = localTimeData(logsout, 'FADO_Id_Assist_Active');
        holdDetails = zeros(size(scenario.HoldWindows, 1), 4);
        for index = 1:size(scenario.HoldWindows, 1)
            hold = scenario.HoldWindows(index, :);
            holdDetails(index, 1) = mean(localWindowData(timeClosed, enClosed, hold) ~= 0);
            holdDetails(index, 2) = mean(localWindowData(timeEnable, enable, hold) ~= 0);
            holdDetails(index, 3) = mean(localWindowData(timeKaf, kafMode, hold) ~= 0);
            holdDetails(index, 4) = mean(localWindowData(timeAssist, idAssist, hold) ~= 0);
        end
        mechanism.Details.HoldRatios = holdDetails;
        mechanism.Details.AllHoldsClosed = all(holdDetails(:, 1) >= 0.95 ...
            & holdDetails(:, 2) >= 0.95 & holdDetails(:, 3) >= 0.50 ...
            & holdDetails(:, 4) >= 0.50);
        mechanism.Pass = metrics.Finite && mechanism.Details.AllHoldsClosed;
        mechanism.Reason = 'three_zero_speed_holds';

    case 'tablei'
        mechanism.Details.TargetSpeedPU = scenario.TargetSpeedPU;
        mechanism.Pass = metrics.CommonStable;
        mechanism.Reason = 'normalized_low_speed_point';

    otherwise
        mechanism.Reason = 'unknown_scenario';
end
end

function tableI = localRunTableISweep(mdl, ids, pmsm, fado, options)
loadFactors = [0 1 2 4 6 8 10]/11.5;
targetSpeeds = 0:0.005:0.025;
runs = repmat(localResultTemplate(), numel(loadFactors), 1);
rows = repmat(struct('LoadFactor', 0, 'LoadNm', 0, ...
    'MinimumStableSpeedPU', NaN, 'StableByTarget', false(size(targetSpeeds))), ...
    numel(loadFactors), 1);

for index = 1:numel(loadFactors)
    scenario = localBuildTableIScenario(loadFactors(index)*pmsm.T_rated, targetSpeeds);
    [runs(index), ~] = localRunWithBandwidthScan(mdl, ids, scenario, pmsm, fado, options);
    rows(index).LoadFactor = loadFactors(index);
    rows(index).LoadNm = loadFactors(index)*pmsm.T_rated;
    rows(index).StableByTarget = localTablePointStability(runs(index), targetSpeeds);
    stableIndex = find(rows(index).StableByTarget, 1, 'first');
    if ~isempty(stableIndex)
        rows(index).MinimumStableSpeedPU = targetSpeeds(stableIndex);
    end
end

tableI = struct('Enabled', true, 'Rows', rows, 'Runs', runs, ...
    'TargetSpeedsPU', targetSpeeds);
end

function scenario = localBuildTableIScenario(loadNm, targetSpeeds)
time = 6.0;
speedKnots = [0 0.25; 3.5 0.25; 6.0 0.0];
windows = zeros(numel(targetSpeeds), 2);
previous = 0.0;
for index = 1:numel(targetSpeeds)
    target = targetSpeeds(index);
    if target ~= previous
        speedKnots(end + 1, :) = [time + 0.1 target]; %#ok<AGROW>
        time = time + 0.1;
    end
    speedKnots(end + 1, :) = [time + 0.65 target]; %#ok<AGROW>
    windows(index, :) = [time + 0.15 time + 0.65];
    time = time + 0.65;
    previous = target;
end
stopTime = time;
scenario = localScenario( ...
    sprintf('TableI_load_%0.6f_Nm', loadNm), 'tablei', stopTime, speedKnots, ...
    [0 0; 3.5 loadNm; stopTime + 0.1 loadNm], ...
    [0 0 0 0 0 0; stopTime + 0.1 0 0 0 0 0], false, windows(end, :));
scenario.TargetSpeedPU = targetSpeeds;
scenario.TargetWindows = windows;
end

function stable = localTablePointStability(run, targetSpeeds)
stable = false(size(targetSpeeds));
if ~isempty(run.Error) || ~isfield(run, 'TablePointStable')
    return;
end

stable = logical(run.TablePointStable(:).');
end

function [stable, pointMetrics] = localCollectTablePointStability(logsout, scenario, pmsm, fado)
count = numel(scenario.TargetSpeedPU);
stable = false(1, count);
pointMetrics = repmat(localFailedMetrics(), count, 1);
for index = 1:count
    pointScenario = scenario;
    pointScenario.EvalWindow = scenario.TargetWindows(index, :);
    pointMetrics(index) = localCollectMetrics(logsout, pointScenario, pmsm, fado);
    stable(index) = pointMetrics(index).CommonStable;
end
end

function parameterCases = localRunParameterErrorCases(mdl, ids, baseScenario, pmsm, fado, options)
cases = [1.2 1.0; 0.8 1.0; 1.0 1.2; 1.0 0.8];
names = {'Rs_plus_20', 'Rs_minus_20', 'Lq_plus_20', 'Lq_minus_20'};
runs = repmat(localResultTemplate(), size(cases, 1), 1);
for index = 1:size(cases, 1)
    scenario = baseScenario;
    scenario.Name = ['Fig10_' names{index}];
    scenario.RsScale = cases(index, 1);
    scenario.LqScale = cases(index, 2);
    [runs(index), ~] = localRunWithBandwidthScan( ...
        mdl, ids, scenario, pmsm, fado, options);
end
parameterCases = struct('Enabled', true, 'Runs', runs);
end

function summary = localSummary(results, tableI, parameterCases)
summary = struct();
summary.AllFigureRunsFinite = all(arrayfun(@(x) x.Metrics.Finite, results));
summary.AllFigureCriteriaPass = all(arrayfun(@(x) x.Pass, results));
summary.FailedFigureNames = string({results(~arrayfun(@(x) x.Pass, results)).Name});
summary.TableSweepEnabled = tableI.Enabled;
summary.ParameterErrorCasesEnabled = parameterCases.Enabled;
end

function signal = localGetSignal(logsout, name)
matches = [];
for index = 1:logsout.numElements
    element = logsout{index};
    if strcmp(element.Name, name)
        matches(end + 1) = index; %#ok<AGROW>
    end
end
assert(~isempty(matches), 'run_fado_paper_reproduction:MissingLog', ...
    'Required logsout signal "%s" is missing.', name);
signal = logsout{matches(end)};
end

function [time, data] = localTimeData(logsout, name)
signal = localGetSignal(logsout, name);
time = double(signal.Values.Time(:));
data = double(signal.Values.Data);
if isvector(data)
    data = data(:);
end
end

function sampled = localSample(sourceTime, sourceData, targetTime)
if isvector(sourceData)
    sampled = interp1(sourceTime, sourceData(:), targetTime, 'previous', 'extrap');
else
    sampled = interp1(sourceTime, sourceData, targetTime, 'previous', 'extrap');
end
end

function data = localWindowData(time, values, window)
mask = time >= window(1) & time <= window(2);
data = values(mask, :);
if isempty(data)
    data = values(end, :);
end
end

function value = localWindowMean(time, values, window)
value = mean(localWindowData(time, values, window), 'all');
end

function value = localWindowDrift(time, values, window)
windowData = localWindowData(time, values, window);
count = numel(windowData);
edgeCount = max(1, floor(0.2*count));
value = abs(mean(windowData(end - edgeCount + 1:end), 'all') ...
    - mean(windowData(1:edgeCount), 'all'));
end

function result = localResultTemplate()
result = struct();
result.Name = '';
result.Kind = '';
result.FastBandwidthHz = NaN;
result.Error = '';
result.LogNames = strings(0, 1);
result.Metrics = localFailedMetrics();
result.Mechanism = localMechanismTemplate();
result.TablePointStable = false(1, 0);
result.TablePointMetrics = repmat(localFailedMetrics(), 0, 1);
result.BandwidthScan = localBandwidthScanTemplate(NaN);
result.Pass = false;
end

function scan = localBandwidthScanTemplate(selectedHz)
scan = struct('Triggered', false, 'CandidatesHz', [], 'Pass', [], ...
    'SelectedHz', selectedHz, 'CandidateMetrics', repmat(localFailedMetrics(), 0, 1), ...
    'CandidateErrors', strings(0, 1));
end

function mechanism = localMechanismTemplate()
details = struct();
details.DhatBefore = NaN;
details.DhatAfter = NaN;
details.DhatExcited = false;
details.Lambda1Drift = zeros(3, 1);
details.KdfOffIncreasesDrift = false;
details.RecoveryFluxFinite = false;
details.KafModeActive = false;
details.IdAssistActive = false;
details.FluxLimited = false;
details.FluxBoundWb = NaN;
details.MaxRotorFluxWb = NaN;
details.FluxBounded = false;
details.ZeroCrossPeakRad = NaN;
details.ClosedLoopAcrossCrossing = false;
details.HoldRatios = zeros(0, 4);
details.AllHoldsClosed = false;
details.TargetSpeedPU = zeros(1, 0);
mechanism = struct('Pass', false, 'Reason', '', 'Details', details);
end

function metrics = localFailedMetrics()
metrics = struct('Finite', false, 'PositionRMSDeg', Inf, 'PositionPeakRad', Inf, ...
    'SpeedErrorRMSPu', Inf, 'SpeedOscillationPu', Inf, 'IqTrackingRMSPu', Inf, ...
    'ClosedLoopRatio', 0, 'EnableRatio', 0, 'CommonStable', false, ...
    'Oscillatory', true, 'DhatMagnitude', [], 'DhatTime', [], ...
    'RotorFluxTime', [], 'RotorFluxMagnitude', [], 'FluxLimit', NaN);
end
