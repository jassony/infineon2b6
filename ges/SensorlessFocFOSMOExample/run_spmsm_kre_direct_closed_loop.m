function simOut = run_spmsm_kre_direct_closed_loop(speedPU, stopTime)
%RUN_SPMSM_KRE_DIRECT_CLOSED_LOOP Run the reproducible SPMSM/KRE startup check.

if nargin < 1
    speedPU = single(0.1);
end
if nargin < 2
    stopTime = 2;
end

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);

if ~bdIsLoaded(modelName)
    open_system(modelFile);
end

directModeBlock = Simulink.ID.getFullName([modelName ':9004']);
speedBlock = Simulink.ID.getFullName([modelName ':9005']);
estimatorBlock = Simulink.ID.getFullName([modelName ':7835']);

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setVariable('KRE_USE_IPMSM', false);
simIn = simIn.setModelParameter('StopTime', num2str(stopTime, 17));
simIn = simIn.setBlockParameter(directModeBlock, 'Value', 'true');
simIn = simIn.setBlockParameter(speedBlock, 'Value', ...
    sprintf('single(%.9g)', double(speedPU)));
simIn = simIn.setBlockParameter(estimatorBlock, 'Value', '3');

simOut = sim(simIn);
end
