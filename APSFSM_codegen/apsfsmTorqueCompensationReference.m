function [stateNext, diagnostics] = apsfsmTorqueCompensationReference( ...
    state, inputs, calibration)
%apsfsmTorqueCompensationReference Reference for production gating/limiting.
%#codegen
%
% Fixed-size single interface:
%   state = [BHat; CHat; ThetaMech; Covariance; SettleCount;
%            ApplyGain; PreviousMode]
%   inputs = [SpeedRef_PU; SpeedFb_PU; BaseD_PU; BaseQ_PU;
%             ControlEligible; ExplicitReset; SystemQLow_PU;
%             SystemQHigh_PU; CurrentMagnitudeLimit_PU]
%   calibration = [Mode; OmegaBase_radps; KHat; Rho_rad; Lambda;
%                  IqCompLow_PU; IqCompHigh_PU; SpeedLow_PU;
%                  SpeedHigh_PU; SettleTicks; RampTicks]
%   diagnostics = [DOut_PU; QOut_PU; IqRaw_PU; IqApplied_PU;
%                  BHat; CHat; ThetaMech; SpeedError_PU; Covariance;
%                  Active; OutputActive; Valid; Status; Clipped;
%                  ApplyGain]
%
% This function is an executable policy oracle for unit/replay tests. The
% generated four-state core remains apsfsmTorqueCompensationStep; firmware
% owns Q15 conversion, control-state gating, and the final current limits.

sampleTime = single(5.0e-4);
zero = single(0.0);
one = single(1.0);

statusIdle = single(0.0);
statusWaitSpeed = single(1.0);
statusShadowValid = single(2.0);
statusApplyValid = single(3.0);
statusParameterInvalid = single(4.0);
statusNumericalInvalid = single(5.0);

stateNext = zeros(7, 1, 'single');
diagnostics = zeros(15, 1, 'single');

baseD = inputs(3);
baseQ = inputs(4);
if isfinite(baseD)
    diagnostics(1) = baseD;
end
if isfinite(baseQ)
    diagnostics(2) = baseQ;
end

mode = calibration(1);
omegaBase = calibration(2);
kHat = calibration(3);
rho = calibration(4);
lambda = calibration(5);
compensationLow = calibration(6);
compensationHigh = calibration(7);
speedLow = calibration(8);
speedHigh = calibration(9);
settleTicks = calibration(10);
rampTicks = calibration(11);

calibrationFinite = true;
for index = 1:11
    calibrationFinite = calibrationFinite && isfinite(calibration(index));
end

modeValid = mode == zero || mode == one || mode == single(2.0);
ticksValid = settleTicks >= one && rampTicks >= one && ...
    settleTicks == floor(settleTicks) && rampTicks == floor(rampTicks);
parameterBoundsValid = omegaBase > zero && kHat > zero && ...
    lambda > zero && lambda < one && compensationLow < zero && ...
    compensationHigh > zero && compensationLow < compensationHigh && ...
    speedLow > zero && speedLow < speedHigh && speedHigh <= one;
convergenceMagnitude = sqrt((single(2.0) * lambda - one) ^ 2 + ...
    (omegaBase * speedHigh * sampleTime) ^ 2);
convergenceValid = isfinite(convergenceMagnitude) && ...
    convergenceMagnitude < one;

if ~calibrationFinite || ~modeValid || ~ticksValid || ...
        ~parameterBoundsValid || ~convergenceValid
    diagnostics(13) = statusParameterInvalid;
    return;
end

minimumCovariance = single(0.5) * kHat * kHat;
if ~isfinite(minimumCovariance) || minimumCovariance <= zero
    diagnostics(13) = statusParameterInvalid;
    return;
end
stateNext(4) = minimumCovariance;

speedReference = inputs(1);
speedFeedback = inputs(2);
controlEligible = inputs(5);
explicitReset = inputs(6);
systemQLow = inputs(7);
systemQHigh = inputs(8);
currentMagnitudeLimit = inputs(9);

inputsFinite = true;
for index = 1:9
    inputsFinite = inputsFinite && isfinite(inputs(index));
end
flagsValid = (controlEligible == zero || controlEligible == one) && ...
    (explicitReset == zero || explicitReset == one);
systemLimitsValid = systemQLow <= zero && systemQHigh >= zero && ...
    systemQLow < systemQHigh && currentMagnitudeLimit > zero;

if ~inputsFinite || ~flagsValid || ~systemLimitsValid
    diagnostics(13) = statusNumericalInvalid;
    return;
end

if mode == zero || explicitReset == one || controlEligible == zero
    diagnostics(13) = statusIdle;
    return;
end

speedInRange = speedReference >= speedLow && ...
    speedReference <= speedHigh && speedFeedback >= speedLow && ...
    speedFeedback <= speedHigh;
if ~speedInRange
    diagnostics(13) = statusWaitSpeed;
    return;
end

policyStateFinite = true;
for index = 1:7
    policyStateFinite = policyStateFinite && isfinite(state(index));
end
if ~policyStateFinite
    diagnostics(13) = statusNumericalInvalid;
    return;
end

settleCount = state(5);
applyGain = state(6);
previousMode = state(7);
policyStateValid = settleCount >= zero && applyGain >= zero && ...
    applyGain <= one && (previousMode == zero || previousMode == one || ...
    previousMode == single(2.0));
if ~policyStateValid
    diagnostics(13) = statusNumericalInvalid;
    return;
end

settleCountNext = settleCount + one;
if settleCountNext > settleTicks
    settleCountNext = settleTicks;
end

stateNext(5) = settleCountNext;
stateNext(7) = mode;
if settleCountNext < settleTicks
    diagnostics(13) = statusWaitSpeed;
    return;
end

coefficientLimit = min(compensationHigh, -compensationLow);
coreParameters = single([omegaBase; kHat; rho; lambda; coefficientLimit]);
coreControl = single([one; zero; zero]);
[coreStateNext, coreDiagnostics] = apsfsmTorqueCompensationStep( ...
    state(1:4), speedReference, speedFeedback, coreParameters, coreControl);

if coreDiagnostics(2) ~= one
    stateNext = zeros(7, 1, 'single');
    stateNext(4) = minimumCovariance;
    diagnostics(13) = coreDiagnostics(3);
    return;
end

iqRaw = coreDiagnostics(1);
speedError = coreDiagnostics(4);
iqApplied = zero;
qOut = baseQ;
clipped = zero;
applyGainNext = zero;

if mode == one
    status = statusShadowValid;
else
    if previousMode ~= single(2.0)
        applyGain = zero;
    end
    completedRampTicks = floor(applyGain * rampTicks + single(0.5));
    applyGainNext = min((completedRampTicks + one) / rampTicks, one);

    boundedCompensation = min(max(iqRaw, compensationLow), ...
        compensationHigh);
    independentlyClipped = boundedCompensation ~= iqRaw;
    requestedCompensation = applyGainNext * boundedCompensation;
    requestedQ = baseQ + requestedCompensation;

    circleRemainder = currentMagnitudeLimit * currentMagnitudeLimit - ...
        baseD * baseD;
    if circleRemainder < zero
        circleRemainder = zero;
    end
    circleQLimit = sqrt(circleRemainder);
    effectiveQLow = max(systemQLow, -circleQLimit);
    effectiveQHigh = min(systemQHigh, circleQLimit);
    qOut = min(max(requestedQ, effectiveQLow), effectiveQHigh);
    externallyClipped = qOut ~= requestedQ;

    if externallyClipped
        coreControl(2) = one;
        [coreStateNext, ~] = apsfsmTorqueCompensationStep( ...
            state(1:4), speedReference, speedFeedback, coreParameters, ...
            coreControl);
    end

    iqApplied = qOut - baseQ;
    if independentlyClipped || externallyClipped
        clipped = one;
    end
    status = statusApplyValid;
end

stateNext(1:4) = coreStateNext;
stateNext(5) = settleCountNext;
stateNext(6) = applyGainNext;
stateNext(7) = mode;

diagnostics(1) = baseD;
diagnostics(2) = qOut;
diagnostics(3) = iqRaw;
diagnostics(4) = iqApplied;
diagnostics(5) = coreStateNext(1);
diagnostics(6) = coreStateNext(2);
diagnostics(7) = coreStateNext(3);
diagnostics(8) = speedError;
diagnostics(9) = coreStateNext(4);
diagnostics(10) = one;
diagnostics(11) = single(mode == single(2.0));
diagnostics(12) = one;
diagnostics(13) = status;
diagnostics(14) = clipped;
diagnostics(15) = applyGainNext;
end
