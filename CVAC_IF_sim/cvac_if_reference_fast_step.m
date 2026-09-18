function [state, output] = cvac_if_reference_fast_step(state, input, param)
%CVAC_IF_REFERENCE_FAST_STEP Independent fast-loop equation reference.
%
% INPUT fields are VdV, VqV, IqA, IqReferenceA, OmegaI0Radps, and
% ThetaRad. PARAM is the cvac structure created by cvac_if_model_init.

if isempty(state)
    state.PowerLowPassW = single(0);
end

Ts = single(param.TsFast);
omegaH = single(2*pi*param.HpfCutoffHz);
alpha = single(exp(-2*pi*param.HpfCutoffHz*param.TsFast));

powerW = single(1.5) * single(input.VqV) * single(input.IqA);
dPowerWps = omegaH * (powerW - state.PowerLowPassW);
state.PowerLowPassW = alpha*state.PowerLowPassW + ...
    (single(1)-alpha)*powerW;

torqueNm = max(single(1.5*param.PolePairs*param.FluxWb) * ...
    abs(single(input.IqReferenceA)), single(param.TorqueMinNm));
deltaOmega = -single(param.Kdp)*dPowerWps/torqueNm;
deltaOmega = min(max(deltaOmega, -single(param.DeltaOmegaMaxRadps)), ...
    single(param.DeltaOmegaMaxRadps));
omegaI = max(single(0), single(input.OmegaI0Radps)+deltaOmega);

thetaValid = omegaI >= single(param.OmegaThetaOnRadps) && ...
    abs(single(input.IqA)) >= single(0.8*param.IMinA) && ...
    all(isfinite(single([input.VdV input.VqV input.IqA])));
thetaRaw = single(0);
thetaEstimate = single(0);
if thetaValid
    thetaRaw = (-omegaI*single(param.LqH)*single(input.IqA) - ...
        single(input.VdV))/(omegaI*single(param.FluxWb));
    thetaValid = isfinite(thetaRaw);
    thetaEstimate = min(max(thetaRaw, ...
        -single(param.ThetaEstimateLimitRad)), ...
        single(param.ThetaEstimateLimitRad));
end

thetaNext = single(mod(double(single(input.ThetaRad)+Ts*omegaI), 2*pi));

output.PowerW = powerW;
output.DPowerWps = dPowerWps;
output.DeltaOmegaRadps = deltaOmega;
output.OmegaIRadps = omegaI;
output.ThetaRawRad = thetaRaw;
output.ThetaEstimateRad = thetaEstimate;
output.ThetaValid = logical(thetaValid);
output.ThetaNextRad = thetaNext;
end

