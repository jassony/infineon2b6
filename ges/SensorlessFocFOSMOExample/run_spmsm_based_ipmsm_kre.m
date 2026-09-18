function simOut = run_spmsm_based_ipmsm_kre(stopTime)
%RUN_SPMSM_BASED_IPMSM_KRE Run the normal state-machine IPMSM/KRE test.

if nargin < 1
    stopTime = 8;
end

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);

if ~bdIsLoaded(modelName)
    open_system(modelFile);
end

directModeBlock = Simulink.ID.getFullName([modelName ':9004']);
estimatorBlock = Simulink.ID.getFullName([modelName ':7835']);

% InitFcn evaluates the selected motor profile before SimulationInput
% variables are applied, so make this test profile visible to InitFcn.
assignin('base', 'KRE_USE_IPMSM', true);
assignin('base', 'KRE_USE_SPMSM_BASED_IPMSM', true);
assignin('base', 'KRE_USE_OBSERVER', true);

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('KRE_USE_IPMSM', true);
simIn = simIn.setVariable('KRE_USE_SPMSM_BASED_IPMSM', true);
simIn = simIn.setVariable('KRE_USE_OBSERVER', true);
simIn = simIn.setModelParameter('StopTime', num2str(stopTime, 17));
simIn = simIn.setBlockParameter(directModeBlock, 'Value', 'false');
simIn = simIn.setBlockParameter(estimatorBlock, 'Value', '3');

simOut = sim(simIn);
end
