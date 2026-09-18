function simOut = run_spmsm_based_ipmsm_kre_hfi(stopTime, amplitudeV, frequencyHz)
%RUN_SPMSM_BASED_IPMSM_KRE_HFI Run KRE/IPMSM with rotating alpha-beta HFI.

if nargin < 1
    stopTime = 8;
end
if nargin < 2
    amplitudeV = single(0.75);
end
if nargin < 3
    frequencyHz = single(400);
end

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);

% InitFcn resolves variant controls before SimulationInput variables apply.
assignin('base', 'KRE_USE_IPMSM', true);
assignin('base', 'KRE_USE_SPMSM_BASED_IPMSM', true);
assignin('base', 'KRE_USE_OBSERVER', true);
assignin('base', 'KRE_HFI_UserEnabled', true);
assignin('base', 'KRE_HFI_Amplitude_V', single(amplitudeV));
assignin('base', 'KRE_HFI_Frequency_Hz', single(frequencyHz));

if ~bdIsLoaded(modelName)
    open_system(modelFile);
end

directModeBlock = Simulink.ID.getFullName([modelName ':9004']);
algorithmSelectorBlock = Simulink.ID.getFullName([modelName ':9063']);

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('KRE_USE_IPMSM', true);
simIn = simIn.setVariable('KRE_USE_SPMSM_BASED_IPMSM', true);
simIn = simIn.setVariable('KRE_USE_OBSERVER', true);
simIn = simIn.setVariable('KRE_HFI_UserEnabled', true);
simIn = simIn.setVariable('KRE_HFI_Amplitude_V', single(amplitudeV));
simIn = simIn.setVariable('KRE_HFI_Frequency_Hz', single(frequencyHz));
simIn = simIn.setModelParameter('StopTime', num2str(stopTime, 17));
simIn = simIn.setBlockParameter(directModeBlock, 'Value', 'false');
simIn = simIn.setBlockParameter(algorithmSelectorBlock, 'Value', '3');

simOut = sim(simIn);
end
