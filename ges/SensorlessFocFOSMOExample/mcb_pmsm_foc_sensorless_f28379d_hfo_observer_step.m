function [speedPU, posPU] = mcb_pmsm_foc_sensorless_f28379d_hfo_observer_step(viFb, params)
%#codegen
% HFO virtual-flux observer for the sensorless FOC model.
% viFb is [Valpha_PU Vbeta_PU Ialpha_PU Ibeta_PU]. The parameter vector is
% [RsEff Ld Lq FluxPM Ts Vbase Ibase Nbase polePairs Kob Kinj SpeedFilterHz].

persistent psiV thetaPrev speedState initialized

if isempty(initialized)
    sample = viFb(1);
    psiV = zeros(2, 1, 'like', sample);
    thetaPrev = zeros(1, 1, 'like', sample);
    speedState = zeros(1, 1, 'like', sample);
    initialized = false;
end

rsEff = params(1);
ld = params(2);
lq = params(3);
fluxPM = params(4);
sampleTime = params(5);
voltageBase = params(6);
currentBase = params(7);
speedBase = params(8);
polePairs = params(9);
kob = params(10);
kinj = params(11);
speedFilterHz = params(12);

voltageAB = [viFb(1); viFb(2)] * voltageBase;
currentAB = [viFb(3); viFb(4)] * currentBase;

% Use the last valid angle for the current-model transform. Before the
% first valid active-flux sample this is zero, which is finite and benign.
thetaUsed = thetaPrev;
cosTheta = cos(thetaUsed);
sinTheta = sin(thetaUsed);
id = cosTheta * currentAB(1) + sinTheta * currentAB(2);
iq = -sinTheta * currentAB(1) + cosTheta * currentAB(2);

% Virtual flux injection from the paper: psi_d_inj = -Kinj*psi_f and
% psi_q_inj = Kinj*psi_f. The injection exists only in the observer.
psiCd = ld * id + fluxPM - kinj * fluxPM;
psiCq = lq * iq + kinj * fluxPM;
psiCAlpha = cosTheta * psiCd - sinTheta * psiCq;
psiCBeta = sinTheta * psiCd + cosTheta * psiCq;
psiC = [psiCAlpha; psiCBeta];

% P-only active-flux observer correction and discrete voltage-model update.
correction = kob * (psiC - psiV);
psiV = psiV + sampleTime * (voltageAB - rsEff * currentAB + correction);
psiA = psiV - lq * currentAB;

activeFluxMagnitude = sqrt(psiA(1) * psiA(1) + psiA(2) * psiA(2));
activeFluxEpsilon = max(single(1e-7), single(1e-4) * abs(fluxPM));
if activeFluxMagnitude > activeFluxEpsilon
    thetaRaw = atan2(psiA(2), psiA(1));

    if initialized
        deltaTheta = thetaRaw - thetaPrev;
        if deltaTheta > pi
            deltaTheta = deltaTheta - 2 * pi;
        elseif deltaTheta < -pi
            deltaTheta = deltaTheta + 2 * pi;
        end

        speedDenominator = max(speedBase, single(1e-6));
        speedRawPU = (deltaTheta / sampleTime) * ...
            (60 / (2 * pi * polePairs * speedDenominator));
        speedFilterGain = 1 - exp(-2 * pi * speedFilterHz * sampleTime);
        speedFilterGain = min(max(speedFilterGain, single(0)), single(1));
        speedState = speedState + speedFilterGain * ...
            (speedRawPU - speedState);
    else
        speedState = zeros(1, 1, 'like', thetaRaw);
        initialized = true;
    end

    thetaPrev = thetaRaw;
end

% Hold thetaPrev and speedState while the active flux is too small. This
% avoids NaN values and +/-pi derivative spikes during startup/reset.
posPU = thetaPrev / (2 * pi);
if posPU < 0
    posPU = posPU + 1;
elseif posPU >= 1
    posPU = posPU - 1;
end
speedPU = speedState;
end
