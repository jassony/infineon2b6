% Run the selector-3 FADO branch without changing the open model.

mdl = 'mcb_pmsm_foc_sensorless_f28379d';
if ~bdIsLoaded(mdl)
    open_system(mdl);
end

selectorPath = Simulink.ID.getFullName([mdl ':7835']);
prototypeSids = [8932 8934 8935 8936];

simIn = Simulink.SimulationInput(mdl);
simIn = simIn.setModelParameter('StopTime', '0.25', ...
    'ReturnWorkspaceOutputs', 'on');
simIn = simIn.setBlockParameter(selectorPath, 'Value', 'uint32(3)');

% The user-visible prototypes remain untouched; comment them only in the
% simulation copy because they are deliberately not wired into the model.
for k = 1:numel(prototypeSids)
    prototypePath = Simulink.ID.getFullName([mdl ':' num2str(prototypeSids(k))]);
    simIn = simIn.setBlockParameter(prototypePath, 'Commented', 'on');
end

simOut = sim(simIn);
logs = simOut.get('logsout');
signalNames = { ...
    'FADO_lambda_alpha1', 'FADO_lambda_beta1', ...
    'FADO_lambda_alpha2', 'FADO_lambda_beta2', ...
    'FADO_Dhat_alpha', 'FADO_Dhat_beta', ...
    'FADO_omega_e_fast', 'FADO_omega_e_slow'};

for k = 1:numel(signalNames)
    signal = logs.get(signalNames{k});
    data = signal.Values.Data;
    if any(~isfinite(double(data(:))))
        error('FADO:ModelSmokeFinite', ...
            'Signal %s contains NaN or Inf.', signalNames{k});
    end
end

fprintf('FADO model smoke test passed: selector=3, %d finite logged states.\n', ...
    numel(signalNames));
