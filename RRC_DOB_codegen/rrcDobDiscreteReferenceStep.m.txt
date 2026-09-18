function [compensatedVoltageDQ_PU, eHatDQ_PU, stateNext, status] = ...
    rrcDobDiscreteReferenceStep(state, previousAppliedVoltageDQ_PU, ...
    currentDQ_PU, rawPiVoltageDQ_PU, parameters, control)
%RRCDOBDISCRETEREFERENCESTEP Single-precision discrete RRC-DOB reference.
%   State order is [xd1; xd2; xd3; xq1; xq2; xq3; qPrev; initialized].
%   The first six states are normalized by Ibase, qPrev is the prior
%   prewarped q, and initialized is exactly zero or one. Each axis uses the
%   closed-form, prewarped-Tustin matrices documented in README.md.
%
%   PARAMETERS is single(8x1), ordered as:
%     [R; Ld; Lq; Vbase; Ibase; Ts; electricalFrequencyHz; cutoffRatio].
%   CONTROL is single(3x1), ordered as:
%     [reset; ramp; outputLimitPU].
%
%   Output order is compensated d/q voltage, disturbance estimate from the
%   input state, updated state, and scalar status. The estimate is published
%   before the state update. The limited correction is
%   clamp(ramp*eHatDQ_PU, +/-outputLimitPU), and is subtracted from raw PI.
%
%   Status values:
%     0 valid                 1 reset applied
%     2 invalid parameter     3 frequency outside 10--750 Hz
%     4 sixth harmonic at Nyquist guard
%     5 nonfinite input, state, coefficient, state update, or output
%     6 correction saturated (the state update remains valid)
%     7 first activation or frequency-ratio reset (zero compensation beat)

compensatedVoltageDQ_PU = zeros(2, 1, 'single');
eHatDQ_PU = zeros(2, 1, 'single');
stateNext = zeros(8, 1, 'single');
status = single(2);

if ~(isa(state, 'single') && isequal(size(state), [8, 1]) && ...
        isa(previousAppliedVoltageDQ_PU, 'single') && ...
        isequal(size(previousAppliedVoltageDQ_PU), [2, 1]) && ...
        isa(currentDQ_PU, 'single') && ...
        isequal(size(currentDQ_PU), [2, 1]) && ...
        isa(rawPiVoltageDQ_PU, 'single') && ...
        isequal(size(rawPiVoltageDQ_PU), [2, 1]) && ...
        isa(parameters, 'single') && isequal(size(parameters), [8, 1]) && ...
        isa(control, 'single') && isequal(size(control), [3, 1]))
    return
end

if ~all(isfinite(rawPiVoltageDQ_PU)) || ~isfinite(control(1))
    status = single(5);
    return
end
compensatedVoltageDQ_PU = rawPiVoltageDQ_PU;

if control(1) ~= single(0)
    status = single(1);
    return
end

if ~all(isfinite(state)) || ...
        ~all(isfinite(previousAppliedVoltageDQ_PU)) || ...
        ~all(isfinite(currentDQ_PU)) || ~all(isfinite(parameters)) || ...
        ~all(isfinite(control))
    status = single(5);
    return
end

resistance_Ohm = parameters(1);
inductanceD_H = parameters(2);
inductanceQ_H = parameters(3);
voltageBase_V = parameters(4);
currentBase_A = parameters(5);
sampleTime_s = parameters(6);
electricalFrequency_Hz = parameters(7);
cutoffRatio = parameters(8);
ramp = control(2);
outputLimit_PU = control(3);

if resistance_Ohm <= single(0) || inductanceD_H <= single(0) || ...
        inductanceQ_H <= single(0) || voltageBase_V <= single(0) || ...
        currentBase_A <= single(0) || sampleTime_s <= single(0) || ...
        cutoffRatio <= single(0) || ramp < single(0) || ...
        ramp > single(1) || outputLimit_PU < single(0)
    status = single(2);
    return
end

if electricalFrequency_Hz <= single(0)
    status = single(3);
    return
end

sixthHarmonic_radps = single(2) * single(pi) * single(6) * ...
    electricalFrequency_Hz;
if sixthHarmonic_radps * sampleTime_s >= single(0.9) * single(pi)
    status = single(4);
    return
end

if electricalFrequency_Hz < single(10) || ...
        electricalFrequency_Hz > single(750)
    status = single(3);
    return
end

halfSampleTime_s = single(0.5) * sampleTime_s;
prewarpedQ = tan(sixthHarmonic_radps * halfSampleTime_s);
if ~isfinite(prewarpedQ) || prewarpedQ <= single(0)
    status = single(5);
    return
end

if state(8) ~= single(0) && state(8) ~= single(1)
    status = single(2);
    return
end

if state(8) == single(0)
    stateNext(7) = prewarpedQ;
    stateNext(8) = single(1);
    status = single(7);
    return
end

previousQ = state(7);
if previousQ <= single(0)
    status = single(2);
    return
end

frequencyRatio = prewarpedQ / previousQ;
if frequencyRatio < single(0.5) || frequencyRatio > single(2)
    stateNext(7) = prewarpedQ;
    stateNext(8) = single(1);
    status = single(7);
    return
end

workingState = state(1:6);
if prewarpedQ ~= previousQ
    ratioSquared = frequencyRatio * frequencyRatio;
    workingState(2) = workingState(2) * ratioSquared;
    workingState(3) = workingState(3) * frequencyRatio;
    workingState(5) = workingState(5) * ratioSquared;
    workingState(6) = workingState(6) * frequencyRatio;
    if ~all(isfinite(workingState))
        status = single(5);
        return
    end
end

[eHatDQ_PU(1), stateNext(1:3), axisDValid] = localAxisStep( ...
    workingState(1:3), previousAppliedVoltageDQ_PU(1), currentDQ_PU(1), ...
    resistance_Ohm, inductanceD_H, voltageBase_V, currentBase_A, ...
    sampleTime_s, prewarpedQ, cutoffRatio);
[eHatDQ_PU(2), stateNext(4:6), axisQValid] = localAxisStep( ...
    workingState(4:6), previousAppliedVoltageDQ_PU(2), currentDQ_PU(2), ...
    resistance_Ohm, inductanceQ_H, voltageBase_V, currentBase_A, ...
    sampleTime_s, prewarpedQ, cutoffRatio);

if ~(axisDValid && axisQValid)
    eHatDQ_PU(:) = single(0);
    stateNext(:) = single(0);
    status = single(5);
    return
end
stateNext(7) = prewarpedQ;
stateNext(8) = single(1);

requestedCorrectionDQ_PU = ramp * eHatDQ_PU;
limitedCorrectionDQ_PU = min(max(requestedCorrectionDQ_PU, ...
    -outputLimit_PU), outputLimit_PU);
compensatedVoltageDQ_PU = rawPiVoltageDQ_PU - limitedCorrectionDQ_PU;

if ~all(isfinite(compensatedVoltageDQ_PU))
    compensatedVoltageDQ_PU = rawPiVoltageDQ_PU;
    eHatDQ_PU(:) = single(0);
    stateNext(:) = single(0);
    status = single(5);
elseif any(limitedCorrectionDQ_PU ~= requestedCorrectionDQ_PU)
    status = single(6);
else
    status = single(0);
end
end

function [eHat_PU, stateNext, valid] = localAxisStep(state, ...
    previousAppliedVoltage_PU, current_PU, resistance_Ohm, inductance_H, ...
    voltageBase_V, currentBase_A, sampleTime_s, q, cutoffRatio)

one = single(1);
two = single(2);
four = single(4);
halfSampleTime_s = single(0.5) * sampleTime_s;
qSquared = q * q;
twoCutoffQ = two * cutoffRatio * q;
r = resistance_Ohm * halfSampleTime_s / inductance_H;
onePlusR = one + r;
denominator = one + twoCutoffQ + qSquared;
commonDenominator = onePlusR * denominator;
voltageInputGain = halfSampleTime_s * voltageBase_V / ...
    (inductance_H * currentBase_A);

a11 = ((one - r) * (one + qSquared) - ...
    twoCutoffQ * onePlusR) / commonDenominator;
a12 = -four * cutoffRatio * q / denominator;
a13 = -four * cutoffRatio * (qSquared - r) / commonDenominator;
a21 = -two * qSquared / commonDenominator;
a22 = (one + twoCutoffQ - qSquared) / denominator;
a23 = two * q * (onePlusR + twoCutoffQ) / commonDenominator;
a31 = -two * q / commonDenominator;
a32 = -two * q / denominator;
a33 = (onePlusR * (one - qSquared) + ...
    twoCutoffQ * (one - r)) / commonDenominator;

b11 = two * voltageInputGain * (one + qSquared) / commonDenominator;
b12 = four * cutoffRatio * q / denominator;
b21 = -two * voltageInputGain * qSquared / commonDenominator;
b22 = two * qSquared / denominator;
b31 = -two * voltageInputGain * q / commonDenominator;
b32 = two * q / denominator;

lambda = inductance_H * currentBase_A / ...
    (voltageBase_V * sampleTime_s);
rho = resistance_Ohm * currentBase_A / voltageBase_V;
outputStateGain = -four * cutoffRatio * q * lambda;
eHat_PU = outputStateGain * state(1) + ...
    outputStateGain * state(2) + two * cutoffRatio * rho * state(3) + ...
    four * cutoffRatio * q * lambda * current_PU;

stateNext = zeros(3, 1, 'single');
stateNext(1) = a11 * state(1) + a12 * state(2) + a13 * state(3) + ...
    b11 * previousAppliedVoltage_PU + b12 * current_PU;
stateNext(2) = a21 * state(1) + a22 * state(2) + a23 * state(3) + ...
    b21 * previousAppliedVoltage_PU + b22 * current_PU;
stateNext(3) = a31 * state(1) + a32 * state(2) + a33 * state(3) + ...
    b31 * previousAppliedVoltage_PU + b32 * current_PU;
valid = isfinite(eHat_PU) && all(isfinite(stateNext));
end
