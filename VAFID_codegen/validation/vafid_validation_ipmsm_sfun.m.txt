function vafid_validation_ipmsm_sfun(block)
%VAFID_VALIDATION_IPMSM_SFUN Discrete dq IPMSM and mechanical MIL plant.
%   Applied voltage is retained in alpha/beta together with its interval
%   angle. At sample k the identifier therefore receives i[k] paired with
%   u[k-1], matching the production VAFID timing contract.

setup(block);
end

function setup(block)
block.NumDialogPrms = 1;
block.DialogPrmsTunable = {'Nontunable'};
block.NumInputPorts = 3;
block.NumOutputPorts = 11;

for portIndex = 1:block.NumInputPorts
    block.InputPort(portIndex).Dimensions = 1;
    block.InputPort(portIndex).DatatypeID = 0;
    block.InputPort(portIndex).Complexity = 'Real';
    block.InputPort(portIndex).DirectFeedthrough = false;
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
stateNames = {'Id_A', 'Iq_A', 'OmegaMechanical_radps', ...
    'ThetaElectrical_rad', 'VoltageAlpha_V', 'VoltageBeta_V', ...
    'VoltageAngle_rad', 'Torque_Nm'};
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
id_A = block.Dwork(1).Data;
iq_A = block.Dwork(2).Data;
omegaMechanical_radps = block.Dwork(3).Data;
thetaElectrical_rad = block.Dwork(4).Data;

cosTheta = cos(thetaElectrical_rad);
sinTheta = sin(thetaElectrical_rad);
currentAlpha_A = cosTheta * id_A - sinTheta * iq_A;
currentBeta_A = sinTheta * id_A + cosTheta * iq_A;

block.OutputPort(1).Data = block.Dwork(5).Data;
block.OutputPort(2).Data = block.Dwork(6).Data;
block.OutputPort(3).Data = currentAlpha_A;
block.OutputPort(4).Data = currentBeta_A;
block.OutputPort(5).Data = block.Dwork(7).Data;
block.OutputPort(6).Data = config.polePairs * omegaMechanical_radps;
block.OutputPort(7).Data = config.fluxPM_Wb + ...
    (config.ld_H - config.lq_H) * id_A;
block.OutputPort(8).Data = omegaMechanical_radps * 30.0 / pi;
block.OutputPort(9).Data = id_A;
block.OutputPort(10).Data = iq_A;
block.OutputPort(11).Data = block.Dwork(8).Data;
end

function update(block)
config = block.DialogPrm(1).Data;
vdApplied_V = block.InputPort(1).Data;
vqApplied_V = block.InputPort(2).Data;
loadTorque_Nm = block.InputPort(3).Data;

id_A = block.Dwork(1).Data;
iq_A = block.Dwork(2).Data;
omegaMechanical_radps = block.Dwork(3).Data;
thetaElectrical_rad = block.Dwork(4).Data;
omegaElectrical_radps = config.polePairs * omegaMechanical_radps;

did_Aps = (vdApplied_V - config.rs_Ohm * id_A + ...
    omegaElectrical_radps * config.lq_H * iq_A) / config.ld_H;
diq_Aps = (vqApplied_V - config.rs_Ohm * iq_A - ...
    omegaElectrical_radps * (config.ld_H * id_A + config.fluxPM_Wb)) / ...
    config.lq_H;
electromagneticTorque_Nm = 1.5 * config.polePairs * ...
    (config.fluxPM_Wb * iq_A + ...
    (config.ld_H - config.lq_H) * id_A * iq_A);
dOmegaMechanical_radps2 = (electromagneticTorque_Nm - loadTorque_Nm - ...
    config.viscousFriction_Nm_per_radps * omegaMechanical_radps) / ...
    config.inertia_kgm2;

cosVoltageAngle = cos(thetaElectrical_rad);
sinVoltageAngle = sin(thetaElectrical_rad);
block.Dwork(5).Data = cosVoltageAngle * vdApplied_V - ...
    sinVoltageAngle * vqApplied_V;
block.Dwork(6).Data = sinVoltageAngle * vdApplied_V + ...
    cosVoltageAngle * vqApplied_V;
block.Dwork(7).Data = thetaElectrical_rad;

block.Dwork(1).Data = id_A + config.sampleTime_s * did_Aps;
block.Dwork(2).Data = iq_A + config.sampleTime_s * diq_Aps;
block.Dwork(3).Data = omegaMechanical_radps + ...
    config.sampleTime_s * dOmegaMechanical_radps2;
thetaElectrical_rad = thetaElectrical_rad + config.sampleTime_s * ...
    omegaElectrical_radps;
block.Dwork(4).Data = atan2(sin(thetaElectrical_rad), ...
    cos(thetaElectrical_rad));
block.Dwork(8).Data = electromagneticTorque_Nm;
end
