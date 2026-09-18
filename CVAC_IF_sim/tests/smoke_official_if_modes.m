function report = smoke_official_if_modes
%SMOKE_OFFICIAL_IF_MODES Verify official variants run via SimulationInput.

root = fileparts(fileparts(mfilename('fullpath')));
addpath(root);
cvac_if_model_init;
mdl = 'mcb_pmsm_foc_sensorless_CVAC_IF_f28379d';
mode = uint8([0; 1]);
inputs = repmat(Simulink.SimulationInput(mdl), numel(mode), 1);

for modeIndex = 1:numel(mode)
    inputs(modeIndex) = Simulink.SimulationInput(mdl);
    inputs(modeIndex) = inputs(modeIndex).setVariable( ...
        'CVAC_IF_Mode', mode(modeIndex));
    inputs(modeIndex) = inputs(modeIndex).setVariable( ...
        'CVAC_MotorProfile', uint8(0));
    inputs(modeIndex) = inputs(modeIndex).setModelParameter( ...
        'StopTime', '0.1');
end

outputs = sim(inputs);
finite = false(numel(mode), 1);
for modeIndex = 1:numel(mode)
    motorSpeed = outputs(modeIndex).logsout.get('MtrSpdTrue').Values;
    finite(modeIndex) = all(isfinite(double(motorSpeed.Data(:))));
end
report = table(double(mode), finite, ...
    'VariableNames', {'CVAC_IF_Mode','Finite'});
disp(report);
assert(all(report.Finite), 'An official I-f mode produced NaN or Inf.');
end

