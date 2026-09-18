function simOut = run_spmsm_based_ipmsm_hfo(stopTime)
%RUN_SPMSM_BASED_IPMSM_HFO Run the normal state-machine IPMSM/HFO test.
% This is the non-zero-speed counterpart to the focused HFO zero-speed test.
% It retains the model's existing start, speed-command, load, and initial-angle
% profiles while selecting HFO and disabling physical KRE/HFI injection.

if nargin < 1
    stopTime = 8;
end
validateattributes(stopTime, {'numeric'}, {'scalar', 'real', 'finite', 'positive'});

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);
if ~bdIsLoaded(modelName)
    open_system(modelFile);
end

directModeBlock = localBlockPath(modelName, 9004);
motorSelectorBlock = localBlockPath(modelName, 9012);
algorithmSelectorBlock = localBlockPath(modelName, 9063);
innerEstimatorBlock = localBlockPath(modelName, 7835);

assignin('base', 'KRE_HFI_UserEnabled', false);
assignin('base', 'KRE_HFI_Amplitude_V', single(0));
assignin('base', 'KRE_HFI_Frequency_Hz', single(0));

% Make the root-level Dashboard selection visible to InitFcn, then update
% the diagram so the profile setup synchronizes the inner Switch Case input.
set_param(directModeBlock, 'Value', 'false');
set_param(motorSelectorBlock, 'Value', '1');
set_param(algorithmSelectorBlock, 'Value', '4');
set_param(modelName, 'SimulationCommand', 'update');
assert(strcmp(strtrim(get_param(innerEstimatorBlock, 'Value')), '4'), ...
    'HFO setup failed to select the internal observer branch.');

hfoPsiD = double(evalin('base', 'HFO_PsiDInjection'));
hfoPsiQ = double(evalin('base', 'HFO_PsiQInjection'));
hfoEq30Margin = double(evalin('base', 'HFO_Eq30Margin'));
assert(hfoPsiQ ~= 0 && hfoPsiD < 0 && hfoEq30Margin > 0, ...
    'HFO setup failed the virtual-flux injection checks.');

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('KRE_HFI_UserEnabled', false);
simIn = simIn.setVariable('KRE_HFI_Amplitude_V', single(0));
simIn = simIn.setVariable('KRE_HFI_Frequency_Hz', single(0));
simIn = simIn.setModelParameter('StopTime', num2str(double(stopTime), 17));
simIn = simIn.setBlockParameter(directModeBlock, 'Value', 'false');
simIn = simIn.setBlockParameter(motorSelectorBlock, 'Value', '1');
simIn = simIn.setBlockParameter(algorithmSelectorBlock, 'Value', '4');
simIn = simIn.setBlockParameter(innerEstimatorBlock, 'Value', '4');

fprintf(['HFO normal run: selector=4, direct closed loop=false, ', ...
    'physical HFI=off, stopTime=%g s\n'], stopTime);
simOut = sim(simIn);
localValidateRun(simOut);
end

function blockPath = localBlockPath(modelName, sid)
blockPath = Simulink.ID.getFullName(sprintf('%s:%d', modelName, sid));
end

function localValidateRun(simOut)
logsout = simOut.logsout;
speedRefTS = localFindTimeseries(logsout, 'Speed_Ref', true);
speedTS = localFindTimeseries(logsout, 'Speed_fb', false);
posTS = localFindTimeseries(logsout, 'Pos_Obs', false);
currentTS = localFindTimeseries(logsout, 'Iab_fb', false);
dutyTS = localFindTimeseries(logsout, 'PWM_Duty_Cycles', false);
closedLoopTS = localFindTimeseries(logsout, 'EnClosedLoop', true);

allFinite = all(isfinite(speedTS.Data(:))) && ...
    all(isfinite(posTS.Data(:))) && ...
    all(isfinite(currentTS.Data(:))) && ...
    all(isfinite(dutyTS.Data(:)));
assert(allFinite, 'HFO normal run produced a nonfinite logged signal.');
assert(max(abs(speedRefTS.Data(:))) > 1e-4, ...
    'HFO normal run did not receive a non-zero speed command.');

tailStart = 0.75 * closedLoopTS.Time(end);
closedLoopTail = closedLoopTS.Data(closedLoopTS.Time >= tailStart);
assert(~isempty(closedLoopTail) && all(closedLoopTail > 0.5), ...
    'HFO normal run did not enter closed loop in the steady-state window.');
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

assert(found, 'HFO normal run is missing logged signal "%s".', signalName);
end
