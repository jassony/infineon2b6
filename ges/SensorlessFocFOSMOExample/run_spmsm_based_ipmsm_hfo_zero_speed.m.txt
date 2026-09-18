function simOut = run_spmsm_based_ipmsm_hfo_zero_speed( ...
    stopTime, initialElectricalDeg, loadTorqueNm)
%RUN_SPMSM_BASED_IPMSM_HFO_ZERO_SPEED Run the focused HFO zero-speed test.
% The Teknic SPMSM-derived IPMSM plant is held at zero speed reference with
% a constant rated load from t = 0. Physical KRE/HFI voltage injection is off.

if nargin < 1
    stopTime = 2;
end
if nargin < 2
    initialElectricalDeg = 70;
end

validateattributes(stopTime, {'numeric'}, {'scalar', 'real', 'finite', 'positive'});
validateattributes(initialElectricalDeg, {'numeric'}, {'scalar', 'real', 'finite'});

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);
if ~bdIsLoaded(modelName)
    open_system(modelFile);
end

% A fresh MATLAB session may not have profile variables yet.
% Update before reading pmsm from the base workspace.
set_param(modelName, 'SimulationCommand', 'update');

pmsmParameters = evalin('base', 'pmsm');
if nargin < 3
    loadTorqueNm = pmsmParameters.T_rated;
end
validateattributes(loadTorqueNm, {'numeric'}, {'scalar', 'real', 'finite'});

% InitFcn creates this vector even though SimulationInput applies selector 4
% after InitFcn. Validate only observer-physics conditions before simulating.
hfoPsiD = double(evalin('base', 'HFO_PsiDInjection'));
hfoPsiQ = double(evalin('base', 'HFO_PsiQInjection'));
hfoEq30Margin = double(evalin('base', 'HFO_Eq30Margin'));
assert(hfoPsiQ ~= 0, 'HFO validation failed: psi_q_inj must be nonzero.');
assert(hfoPsiD < 0, 'HFO validation failed: psi_d_inj must be negative.');
assert(hfoEq30Margin > 0, 'HFO validation failed: equation (30) margin must be positive.');

% Clear any state left by the KRE/HFI runner without changing the model.
assignin('base', 'KRE_HFI_UserEnabled', false);
assignin('base', 'KRE_HFI_Amplitude_V', single(0));
assignin('base', 'KRE_HFI_Frequency_Hz', single(0));

polePairs = double(pmsmParameters.p);
thetaInitMechanicalRad = deg2rad(double(initialElectricalDeg)) / polePairs;

directModeBlock = Simulink.ID.getFullName([modelName ':9004']);
manualSpeedBlock = Simulink.ID.getFullName([modelName ':9005']);
motorSelectorBlock = Simulink.ID.getFullName([modelName ':9012']);
algorithmSelectorBlock = Simulink.ID.getFullName([modelName ':9063']);
innerEstimatorBlock = Simulink.ID.getFullName([modelName ':7835']);
ipmsmPlantBlock = Simulink.ID.getFullName([modelName ':9002']);
loadTorqueBlock = Simulink.ID.getFullName([modelName ':8930']);

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('KRE_HFI_UserEnabled', false);
simIn = simIn.setVariable('KRE_HFI_Amplitude_V', single(0));
simIn = simIn.setVariable('KRE_HFI_Frequency_Hz', single(0));
simIn = simIn.setModelParameter('StopTime', num2str(stopTime, 17));
simIn = simIn.setBlockParameter(directModeBlock, 'Value', 'true');
simIn = simIn.setBlockParameter(manualSpeedBlock, 'Value', 'single(0)');
simIn = simIn.setBlockParameter(motorSelectorBlock, 'Value', '1');
simIn = simIn.setBlockParameter(algorithmSelectorBlock, 'Value', '4');
simIn = simIn.setBlockParameter(innerEstimatorBlock, 'Value', '4');
simIn = simIn.setBlockParameter(ipmsmPlantBlock, 'theta_init', ...
    num2str(thetaInitMechanicalRad, 17));
simIn = simIn.setBlockParameter(loadTorqueBlock, 'Time', '0');
simIn = simIn.setBlockParameter(loadTorqueBlock, 'Before', ...
    num2str(double(loadTorqueNm), 17));
simIn = simIn.setBlockParameter(loadTorqueBlock, 'After', ...
    num2str(double(loadTorqueNm), 17));

fprintf('HFO zero-speed validation: psi_d_inj=%g, psi_q_inj=%g, eq30 margin=%g\n', ...
    hfoPsiD, hfoPsiQ, hfoEq30Margin);
simOut = sim(simIn);
localValidateRun(simOut);
end

function localValidateRun(simOut)
logsout = simOut.logsout;
speedTS = localFindTimeseries(logsout, 'Speed_fb', false);
posTS = localFindTimeseries(logsout, 'Pos_Obs', false);
currentTS = localFindTimeseries(logsout, 'Iab_fb', false);
dutyTS = localFindTimeseries(logsout, 'PWM_Duty_Cycles', false);
closedLoopTS = localFindTimeseries(logsout, 'EnClosedLoop', true);

allFinite = all(isfinite(speedTS.Data(:))) && ...
    all(isfinite(posTS.Data(:))) && ...
    all(isfinite(currentTS.Data(:))) && ...
    all(isfinite(dutyTS.Data(:)));
assert(allFinite, 'HFO zero-speed test produced a nonfinite logged signal.');

tailStart = 0.75 * speedTS.Time(end);
speedTail = speedTS.Data(speedTS.Time >= tailStart);
assert(~isempty(speedTail), 'HFO zero-speed test has no steady-state samples.');
assert(max(abs(speedTail)) <= 0.05, ...
    'HFO zero-speed test did not settle below 0.05 PU speed feedback.');

closedLoopTail = closedLoopTS.Data(closedLoopTS.Time >= tailStart);
assert(~isempty(closedLoopTail) && all(closedLoopTail > 0.5), ...
    'HFO zero-speed test left direct closed loop during the steady-state window.');
end

function ts = localFindTimeseries(logsout, signalName, preferActive)
found = false;
bestPeak = -inf;
ts = timeseries;

for index = 1:logsout.numElements
    element = logsout{index};
    if strcmp(element.Name, signalName) && isa(element.Values, 'timeseries')
        candidate = element.Values;
        peak = max(abs(candidate.Data(:)));
        if ~found || (~preferActive || peak > bestPeak)
            ts = candidate;
            bestPeak = peak;
            found = true;
        end
    end
end

assert(found, 'HFO zero-speed test is missing logged signal "%s".', signalName);
end
