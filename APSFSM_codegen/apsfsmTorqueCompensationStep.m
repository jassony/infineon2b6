function [stateNext, diagnostics] = apsfsmTorqueCompensationStep( ...
    state, speedReferencePU, speedFeedbackPU, parameters, control)
%apsfsmTorqueCompensationStep Execute one 2 kHz APSFSM learning sample.
%#codegen
%
% Fixed-size single interface:
%   state      = [BHat; CHat; ThetaMech; Covariance]
%   parameters = [OmegaBase_radps; KHat; Rho_rad; Lambda; CoeffLimit_PU]
%   control    = [LearnEnable; FreezeAdaptation; Reset]
%   diagnostics= [IqRaw_PU; Valid; Status; SpeedError_PU]
%
% Status follows the firmware diagnostic contract where applicable:
%   0 = idle/reset, 2 = learning valid, 4 = parameter invalid,
%   5 = numerical input/state invalid.
%
% The output deliberately uses the pre-update B/C/theta state. Theta is
% then integrated, and the new theta/covariance are used to update B/C.
% When FreezeAdaptation is true, B/C/covariance are held while theta still
% advances. Gating, settle timing, modes, and dq limiting are handled by the
% production policy layer and are not duplicated in this four-state core.

sampleTime = single(5.0e-4);
twoPi = single(2.0 * pi);
half = single(0.5);

statusIdle = single(0.0);
statusLearningValid = single(2.0);
statusParameterInvalid = single(4.0);
statusNumericalInvalid = single(5.0);

stateNext = zeros(4, 1, 'single');
diagnostics = zeros(4, 1, 'single');
diagnostics(3) = statusIdle;

omegaBase = parameters(1);
kHat = parameters(2);
rho = parameters(3);
lambda = parameters(4);
coefficientLimit = parameters(5);

parameterValuesFinite = isfinite(omegaBase) && isfinite(kHat) && ...
    isfinite(rho) && isfinite(lambda) && isfinite(coefficientLimit);
parametersValid = parameterValuesFinite && omegaBase > single(0.0) && ...
    kHat > single(0.0) && lambda > single(0.0) && ...
    lambda < single(1.0) && coefficientLimit > single(0.0);

if ~parametersValid
    diagnostics(3) = statusParameterInvalid;
    return;
end

minimumCovariance = half * kHat * kHat;
if ~isfinite(minimumCovariance) || minimumCovariance <= single(0.0)
    diagnostics(3) = statusParameterInvalid;
    return;
end

resetState = zeros(4, 1, 'single');
resetState(4) = minimumCovariance;
stateNext = resetState;

learnEnable = control(1);
freezeAdaptation = control(2);
reset = control(3);
controlFinite = isfinite(learnEnable) && isfinite(freezeAdaptation) && ...
    isfinite(reset);
controlDiscrete = (learnEnable == single(0.0) || ...
    learnEnable == single(1.0)) && ...
    (freezeAdaptation == single(0.0) || ...
    freezeAdaptation == single(1.0)) && ...
    (reset == single(0.0) || reset == single(1.0));

if ~controlFinite || ~controlDiscrete || ...
        ~isfinite(speedReferencePU) || ~isfinite(speedFeedbackPU)
    diagnostics(3) = statusNumericalInvalid;
    return;
end

if reset == single(1.0) || learnEnable == single(0.0)
    return;
end

bHat = state(1);
cHat = state(2);
thetaMech = state(3);
covariance = state(4);
stateFinite = isfinite(bHat) && isfinite(cHat) && ...
    isfinite(thetaMech) && isfinite(covariance);
if stateFinite && covariance >= single(0.0) && ...
        covariance < minimumCovariance
    % The independent wrapper uses a parameter-independent zero Unit Delay
    % initial condition. Recover the source covariance initialization on
    % the first enabled sample and after a live KHat increase.
    covariance = minimumCovariance;
end
coefficientMagnitude = sqrt(bHat * bHat + cHat * cHat);
stateInvariantValid = covariance >= minimumCovariance && ...
    isfinite(coefficientMagnitude) && ...
    coefficientMagnitude <= coefficientLimit * single(1.00001);

if ~stateFinite || ~stateInvariantValid
    diagnostics(3) = statusNumericalInvalid;
    return;
end

thetaMech = localWrapZeroToTwoPi(thetaMech, twoPi);
speedError = speedReferencePU - speedFeedbackPU;
iqRaw = bHat * sin(thetaMech) + cHat * cos(thetaMech);
thetaNext = localWrapZeroToTwoPi(thetaMech + ...
    sampleTime * omegaBase * speedFeedbackPU, twoPi);

if freezeAdaptation == single(1.0)
    bNext = bHat;
    cNext = cHat;
    covarianceNext = covariance;
else
    covarianceNext = lambda * covariance + minimumCovariance;
    if covarianceNext < minimumCovariance
        covarianceNext = minimumCovariance;
    end

    rhoWrapped = localWrapMinusPiToPi(rho, twoPi);
    phase = thetaNext + rhoWrapped;
    bNext = bHat + kHat * sin(phase) * speedError / covarianceNext;
    cNext = cHat + kHat * cos(phase) * speedError / covarianceNext;

    candidateMagnitude = sqrt(bNext * bNext + cNext * cNext);
    if candidateMagnitude > coefficientLimit
        projectionScale = coefficientLimit / candidateMagnitude;
        bNext = bNext * projectionScale;
        cNext = cNext * projectionScale;
    end
end

nextValuesFinite = isfinite(iqRaw) && isfinite(speedError) && ...
    isfinite(thetaNext) && isfinite(bNext) && isfinite(cNext) && ...
    isfinite(covarianceNext);
if ~nextValuesFinite
    stateNext = resetState;
    diagnostics(3) = statusNumericalInvalid;
    return;
end

stateNext(1) = bNext;
stateNext(2) = cNext;
stateNext(3) = thetaNext;
stateNext(4) = covarianceNext;
diagnostics(1) = iqRaw;
diagnostics(2) = single(1.0);
diagnostics(3) = statusLearningValid;
diagnostics(4) = speedError;
end

function wrapped = localWrapZeroToTwoPi(angle, twoPi)
wrapped = angle - floor(angle / twoPi) * twoPi;
if wrapped >= twoPi
    wrapped = single(0.0);
elseif wrapped < single(0.0)
    wrapped = wrapped + twoPi;
end
end

function wrapped = localWrapMinusPiToPi(angle, twoPi)
piSingle = single(pi);
wrapped = localWrapZeroToTwoPi(angle + piSingle, twoPi) - piSingle;
end
