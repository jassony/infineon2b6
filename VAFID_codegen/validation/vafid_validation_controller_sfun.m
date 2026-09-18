function vafid_validation_controller_sfun(block)
%VAFID_VALIDATION_CONTROLLER_SFUN Two-rate speed/current controller for MIL.
%   The speed loop runs at 2 kHz and the current loop at 20 kHz. VAFID probe
%   currents are added to the d/q references before the PI current loop, so
%   the plant current is a closed-loop response rather than a prescribed
%   analytical waveform.

setup(block);
end

function setup(block)
block.NumDialogPrms = 1;
block.DialogPrmsTunable = {'Nontunable'};
block.NumInputPorts = 7;
block.NumOutputPorts = 8;

for portIndex = 1:block.NumInputPorts
    block.InputPort(portIndex).Dimensions = 1;
    block.InputPort(portIndex).DatatypeID = 0;
    block.InputPort(portIndex).Complexity = 'Real';
    block.InputPort(portIndex).DirectFeedthrough = true;
end

for portIndex = 1:block.NumOutputPorts
    block.OutputPort(portIndex).Dimensions = 1;
    block.OutputPort(portIndex).DatatypeID = 0;
    block.OutputPort(portIndex).Complexity = 'Real';
end

block.SampleTimes = [50e-6, 0.0];
block.SimStateCompliance = 'DefaultSimState';

block.RegBlockMethod('PostPropagationSetup', @postPropagationSetup);
block.RegBlockMethod('InitializeConditions', @initializeConditions);
block.RegBlockMethod('Outputs', @outputs);
block.RegBlockMethod('Update', @update);
end

function postPropagationSetup(block)
stateNames = {'SpeedIntegral_A', 'IqBase_A', 'IdIntegral_V', ...
    'IqIntegral_V', 'SpeedDividerCount', 'EffectiveSpeedRef_rpm'};
block.NumDworks = numel(stateNames);
for stateIndex = 1:numel(stateNames)
    block.Dwork(stateIndex).Name = stateNames{stateIndex};
    block.Dwork(stateIndex).Dimensions = 1;
    block.Dwork(stateIndex).DatatypeID = 0;
    block.Dwork(stateIndex).Complexity = 'Real';
    block.Dwork(stateIndex).UsedAsDiscState = true;
end
end

function initializeConditions(block)
for stateIndex = 1:block.NumDworks
    block.Dwork(stateIndex).Data = 0.0;
end
end

function outputs(block)
config = block.DialogPrm(1).Data;
speedReference_rpm = block.Dwork(6).Data;
speedActual_rpm = block.InputPort(2).Data;
id_A = block.InputPort(3).Data;
iq_A = block.InputPort(4).Data;
omegaElectrical_radps = block.InputPort(5).Data;
probeD_PU = block.InputPort(6).Data;
probeQ_PU = block.InputPort(7).Data;

idReference_A = config.idBase_A + config.currentBase_A * probeD_PU;
iqReference_A = block.Dwork(2).Data + config.currentBase_A * probeQ_PU;
idError_A = idReference_A - id_A;
iqError_A = iqReference_A - iq_A;

vdUnsaturated_V = config.currentKpD_V_per_A * idError_A + ...
    block.Dwork(3).Data - omegaElectrical_radps * config.nominalLq_H * iq_A;
vqUnsaturated_V = config.currentKpQ_V_per_A * iqError_A + ...
    block.Dwork(4).Data + omegaElectrical_radps * ...
    (config.nominalLd_H * id_A + config.nominalFluxPM_Wb);

voltageMagnitude_V = hypot(vdUnsaturated_V, vqUnsaturated_V);
if voltageMagnitude_V > config.maximumVoltage_V
    voltageScale = config.maximumVoltage_V / voltageMagnitude_V;
else
    voltageScale = 1.0;
end

block.OutputPort(1).Data = voltageScale * vdUnsaturated_V;
block.OutputPort(2).Data = voltageScale * vqUnsaturated_V;
block.OutputPort(3).Data = idReference_A;
block.OutputPort(4).Data = iqReference_A;
block.OutputPort(5).Data = min(voltageMagnitude_V / ...
    config.maximumVoltage_V, 1.0);
block.OutputPort(6).Data = hypot(idError_A, iqError_A);
block.OutputPort(7).Data = abs(speedReference_rpm - speedActual_rpm);
block.OutputPort(8).Data = speedReference_rpm;
end

function update(block)
config = block.DialogPrm(1).Data;
speedCommand_rpm = block.InputPort(1).Data;
speedActual_rpm = block.InputPort(2).Data;
id_A = block.InputPort(3).Data;
iq_A = block.InputPort(4).Data;
omegaElectrical_radps = block.InputPort(5).Data;
probeD_PU = block.InputPort(6).Data;
probeQ_PU = block.InputPort(7).Data;

dividerCount = block.Dwork(5).Data + 1.0;
if dividerCount >= config.speedLoopDivider
    speedLoopSampleTime_s = config.sampleTime_s * config.speedLoopDivider;
    maximumSpeedStep_rpm = config.speedSlewRate_rpmps * speedLoopSampleTime_s;
    speedReference_rpm = block.Dwork(6).Data + clamp( ...
        speedCommand_rpm - block.Dwork(6).Data, -maximumSpeedStep_rpm, ...
        maximumSpeedStep_rpm);
    block.Dwork(6).Data = speedReference_rpm;
    speedError_rpm = speedReference_rpm - speedActual_rpm;
    speedIntegral_A = block.Dwork(1).Data;
    iqUnsaturated_A = config.speedKp_A_per_rpm * speedError_rpm + ...
        speedIntegral_A;
    iqBase_A = clamp(iqUnsaturated_A, config.iqMinimum_A, ...
        config.iqMaximum_A);

    drivesFurtherIntoSaturation = ...
        (iqUnsaturated_A > config.iqMaximum_A && speedError_rpm > 0.0) || ...
        (iqUnsaturated_A < config.iqMinimum_A && speedError_rpm < 0.0);
    if ~drivesFurtherIntoSaturation
        speedIntegral_A = speedIntegral_A + ...
            config.speedKi_A_per_rpm_s * speedLoopSampleTime_s * speedError_rpm;
        speedIntegral_A = clamp(speedIntegral_A, config.iqMinimum_A, ...
            config.iqMaximum_A);
    end

    block.Dwork(1).Data = speedIntegral_A;
    block.Dwork(2).Data = iqBase_A;
    dividerCount = 0.0;
end
block.Dwork(5).Data = dividerCount;

idReference_A = config.idBase_A + config.currentBase_A * probeD_PU;
iqReference_A = block.Dwork(2).Data + config.currentBase_A * probeQ_PU;
idError_A = idReference_A - id_A;
iqError_A = iqReference_A - iq_A;

vdUnsaturated_V = config.currentKpD_V_per_A * idError_A + ...
    block.Dwork(3).Data - omegaElectrical_radps * config.nominalLq_H * iq_A;
vqUnsaturated_V = config.currentKpQ_V_per_A * iqError_A + ...
    block.Dwork(4).Data + omegaElectrical_radps * ...
    (config.nominalLd_H * id_A + config.nominalFluxPM_Wb);
voltageMagnitude_V = hypot(vdUnsaturated_V, vqUnsaturated_V);
if voltageMagnitude_V > config.maximumVoltage_V
    voltageScale = config.maximumVoltage_V / voltageMagnitude_V;
else
    voltageScale = 1.0;
end
vdApplied_V = voltageScale * vdUnsaturated_V;
vqApplied_V = voltageScale * vqUnsaturated_V;

block.Dwork(3).Data = block.Dwork(3).Data + ...
    config.currentKi_V_per_A_s * config.sampleTime_s * idError_A + ...
    config.currentAntiWindup_per_s * config.sampleTime_s * ...
    (vdApplied_V - vdUnsaturated_V);
block.Dwork(4).Data = block.Dwork(4).Data + ...
    config.currentKi_V_per_A_s * config.sampleTime_s * iqError_A + ...
    config.currentAntiWindup_per_s * config.sampleTime_s * ...
    (vqApplied_V - vqUnsaturated_V);
block.Dwork(3).Data = clamp(block.Dwork(3).Data, ...
    -config.maximumVoltage_V, config.maximumVoltage_V);
block.Dwork(4).Data = clamp(block.Dwork(4).Data, ...
    -config.maximumVoltage_V, config.maximumVoltage_V);
end

function value = clamp(value, minimumValue, maximumValue)
value = min(max(value, minimumValue), maximumValue);
end
