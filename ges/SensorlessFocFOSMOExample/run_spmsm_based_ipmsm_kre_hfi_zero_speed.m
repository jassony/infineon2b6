function simOut = run_spmsm_based_ipmsm_kre_hfi_zero_speed( ...
    stopTime, initialElectricalDeg, amplitudeV, frequencyHz, loadTorqueNm)
%RUN_SPMSM_BASED_IPMSM_KRE_HFI_ZERO_SPEED Run zero-speed KRE/IPMSM HFI test.
%   The plant starts with an electrical-angle offset while the KRE observer
%   retains its zero-state initialization. The test uses manual direct
%   closed loop so a zero speed reference does not remain in Stateflow Halt.
%   loadTorqueNm is a constant motor-side load torque applied from t = 0.

if nargin < 1
    stopTime = 2;
end
if nargin < 2
    initialElectricalDeg = 70;
end
if nargin < 3
    amplitudeV = single(0.75);
end
if nargin < 4
    frequencyHz = single(400);
end
if nargin < 5
    loadTorqueNm = 0;
end

validateattributes(stopTime, {'numeric'}, {'scalar', 'real', 'finite', 'positive'});
validateattributes(initialElectricalDeg, {'numeric'}, {'scalar', 'real', 'finite'});
validateattributes(amplitudeV, {'numeric'}, {'scalar', 'real', 'finite', 'nonnegative'});
validateattributes(frequencyHz, {'numeric'}, {'scalar', 'real', 'finite', 'positive'});
validateattributes(loadTorqueNm, {'numeric'}, {'scalar', 'real', 'finite'});

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);

if ~bdIsLoaded(modelName)
    open_system(modelFile);
end

% Make the HFI configuration available when the model InitFcn resolves
% variant controls. The SimulationInput repeats these values for the run.
assignin('base', 'KRE_HFI_UserEnabled', true);
assignin('base', 'KRE_HFI_Amplitude_V', single(amplitudeV));
assignin('base', 'KRE_HFI_Frequency_Hz', single(frequencyHz));

pmsmParameters = evalin('base', 'pmsm');
polePairs = double(pmsmParameters.p);
thetaInitMechanicalRad = deg2rad(double(initialElectricalDeg)) / polePairs;

directModeBlock = Simulink.ID.getFullName([modelName ':9004']);
manualSpeedBlock = Simulink.ID.getFullName([modelName ':9005']);
motorSelectorBlock = Simulink.ID.getFullName([modelName ':9012']);
algorithmSelectorBlock = Simulink.ID.getFullName([modelName ':9063']);
ipmsmPlantBlock = Simulink.ID.getFullName([modelName ':9002']);
loadTorqueBlock = Simulink.ID.getFullName([modelName ':8930']);

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('KRE_HFI_UserEnabled', true);
simIn = simIn.setVariable('KRE_HFI_Amplitude_V', single(amplitudeV));
simIn = simIn.setVariable('KRE_HFI_Frequency_Hz', single(frequencyHz));
simIn = simIn.setModelParameter('StopTime', num2str(stopTime, 17));
simIn = simIn.setBlockParameter(directModeBlock, 'Value', 'true');
simIn = simIn.setBlockParameter(manualSpeedBlock, 'Value', 'single(0)');
simIn = simIn.setBlockParameter(motorSelectorBlock, 'Value', '1');
simIn = simIn.setBlockParameter(algorithmSelectorBlock, 'Value', '3');
simIn = simIn.setBlockParameter(ipmsmPlantBlock, 'theta_init', ...
    num2str(thetaInitMechanicalRad, 17));
simIn = simIn.setBlockParameter(loadTorqueBlock, 'Time', '0');
simIn = simIn.setBlockParameter(loadTorqueBlock, 'Before', ...
    num2str(double(loadTorqueNm), 17));
simIn = simIn.setBlockParameter(loadTorqueBlock, 'After', ...
    num2str(double(loadTorqueNm), 17));

simOut = sim(simIn);
end
