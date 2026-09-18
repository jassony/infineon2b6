function report = run_cvac_if_robustness
%RUN_CVAC_IF_ROBUSTNESS Sweep estimated flux and Lq at rated load.

root = fileparts(fileparts(mfilename('fullpath')));
addpath(root);
cvac_if_model_init;
mdl = 'mcb_pmsm_foc_sensorless_CVAC_IF_f28379d';

caseName = ["Flux50"; "Flux100"; "Flux150"; ...
    "Lq70"; "Lq100"; "Lq130"];
fluxScale = [0.5; 1.0; 1.5; 1.0; 1.0; 1.0];
lqScale = [1.0; 1.0; 1.0; 0.7; 1.0; 1.3];
stopTimeS = [2; 2; 2; 5; 2; 5];
caseCount = numel(caseName);
inputs = repmat(Simulink.SimulationInput(mdl), caseCount, 1);

for caseIndex = 1:caseCount
    cvacCase = cvac;
    cvacCase.FluxWb = cvac.FluxWb*fluxScale(caseIndex);
    cvacCase.LqH = cvac.LqH*lqScale(caseIndex);
    inputs(caseIndex) = Simulink.SimulationInput(mdl);
    inputs(caseIndex) = inputs(caseIndex).setVariable( ...
        'CVAC_IF_Mode', uint8(2));
    inputs(caseIndex) = inputs(caseIndex).setVariable( ...
        'CVAC_MotorProfile', uint8(1));
    inputs(caseIndex) = inputs(caseIndex).setVariable( ...
        'CVAC_LoadInitial', 0);
    inputs(caseIndex) = inputs(caseIndex).setVariable( ...
        'CVAC_LoadFinal', 1);
    inputs(caseIndex) = inputs(caseIndex).setVariable( ...
        'CVAC_LoadStepTime', 0.52);
    inputs(caseIndex) = inputs(caseIndex).setVariable( ...
        'CVAC_LoadConstant', 1);
    inputs(caseIndex) = inputs(caseIndex).setVariable('cvac', cvacCase);
    inputs(caseIndex) = inputs(caseIndex).setModelParameter( ...
        'StopTime', num2str(stopTimeS(caseIndex)));
end

outputs = sim(inputs);
report = table(caseName, fluxScale, lqScale, zeros(caseCount,1), ...
    zeros(caseCount,1), nan(caseCount,1), false(caseCount,1), ...
    false(caseCount,1), 'VariableNames', {'Case','FluxScale','LqScale', ...
    'FinalState','AbortCode','HandoffTime','Finite','Passed'});

for caseIndex = 1:caseCount
    diagnostics = outputs(caseIndex).logsout.get( ...
        'CVAC_Diagnostics').Values;
    diagnosticData = squeeze(double(diagnostics.Data));
    handoff = outputs(caseIndex).logsout.get( ...
        'CVAC_EnableSpeedLoop').Values;
    handoffIndex = find(logical(handoff.Data(:)), 1, 'first');
    if ~isempty(handoffIndex)
        report.HandoffTime(caseIndex) = handoff.Time(handoffIndex);
    end
    report.FinalState(caseIndex) = diagnosticData(1,end);
    report.AbortCode(caseIndex) = diagnosticData(12,end);
    report.Finite(caseIndex) = all(isfinite(diagnosticData(:)));
    report.Passed(caseIndex) = report.Finite(caseIndex) && ...
        report.FinalState(caseIndex) == 7 && ...
        report.AbortCode(caseIndex) == 0;
end

disp(report);
assert(all(report.Finite), 'A robustness run produced NaN or Inf.');
assert(all(report.Passed), ...
    'At least one paper-parameter robustness case failed startup.');
end

