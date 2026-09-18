function [nextState, activeFluxAlphaWb, activeFluxBetaWb, status] = ...
    kre_ekf_kre_step(state, enable, viFbPU, params, reset, pllParams)
%KRE_EKF_KRE_STEP Explicit-state KRE active-flux estimator for FluxEKF.
%
% State layout (18x1):
% [h2Vri(2); h2I(2); h2Reg; h2D; qState(:); yState(2); lambdaHat(2);
%  pllTheta; pllOmegaInt; speedFilt; pllInitialized]. The final four PLL
% slots are retained for state-vector compatibility but are not updated.
% viFbPU is [ValphaPU; VbetaPU; IalphaPU; IbetaPU]. params uses the
% project KRE order [Rs; Ld; Lq; psiM; Ts; Vbase; Ibase; baseRPM;
% polePairs; alpha; A; gamma; sigmaEpsilon; speedFilterHz].
%
% activeFluxAlphaWb and activeFluxBetaWb are xHat = lambdaHat - Lq*iAB
% from the pre-update KRE state. status is 0 when disabled, 1 when xHat is
% finite and has magnitude at least sigmaEpsilon, and 2 otherwise. FluxEKF
% must consume the vector only when status is 1.

if numel(state) ~= 18 || numel(viFbPU) ~= 4 || numel(params) ~= 14 ...
        || numel(pllParams) ~= 2 || numel(enable) ~= 1 || numel(reset) ~= 1
    error('kre_ekf_kre_step:InvalidInputSize', ...
        'Expected state(18), viFbPU(4), params(14), pllParams(2), and scalar enable/reset.');
end

oldState = reshape(state, 18, 1);
% Keep every arithmetic input in the state precision. This lets the same
% step function serve both the single-precision KRE chart and double-
% precision offline calls without code-generation type conflicts.
viFbPU = cast(reshape(viFbPU, 4, 1), 'like', oldState);
params = cast(reshape(params, 14, 1), 'like', oldState);
pllParams = cast(reshape(pllParams, 2, 1), 'like', oldState); %#ok<NASGU>

zero = zeros(1, 1, 'like', oldState);
one = zero + 1;
activeFluxAlphaWb = zero;
activeFluxBetaWb = zero;
status = uint8(0);

% The generated model resets on an edge; use reset as a one-sample pulse.
if reset ~= 0
    oldState = zeros(18, 1, 'like', oldState);
end

% Disabled KRE preserves its observer state and reports deterministic idle data.
if enable == 0
    nextState = oldState;
    return
end

resistance = params(1);
ld = params(2);
lq = params(3);
psiM = params(4);
sampleTime = params(5);
voltageBase = params(6);
currentBase = params(7);
alpha = params(10);
aGain = params(11);
gamma = params(12);
sigmaEpsilon = params(13);

h2Vri = oldState(1:2);
h2I = oldState(3:4);
h2Reg = oldState(5);
h2D = oldState(6);
qState = [oldState(7), oldState(9); oldState(8), oldState(10)];
yState = oldState(11:12);
lambdaHat = oldState(13:14);

voltageAB = [viFbPU(1); viFbPU(2)] * voltageBase;
currentAB = [viFbPU(3); viFbPU(4)] * currentBase;
l0 = ld - lq;

% Exact zero-order-hold discretization of H2 = alpha / (p + alpha).
h2Gain = one - exp(-alpha * sampleTime);
vri = voltageAB - resistance * currentAB;
h2VriNext = h2Vri + h2Gain * (vri - h2Vri);
h2INext = h2I + h2Gain * (currentAB - h2I);
h1I = alpha * (currentAB - h2INext);

omega1 = h2VriNext - lq * h1I;
omega2 = omega1 - l0 * h1I;
phi = omega1 + omega2;

regressionProduct = omega2(1) * omega1(1) + omega2(2) * omega1(2);
h2RegNext = h2Reg + h2Gain * (regressionProduct - h2Reg);
regressionY = l0 * (h2INext(1) * omega1(1) + h2INext(2) * omega1(2)) ...
    + (omega1(1) * omega1(1) + omega1(2) * omega1(2)) / alpha ...
    + h2RegNext / alpha;

% xHat is the active-flux vector handed to FluxEKF, before lambdaHat update.
xHat = lambdaHat - lq * currentAB;
xNorm = sqrt(xHat(1) * xHat(1) + xHat(2) * xHat(2));
if xNorm >= sigmaEpsilon
    sigmaX = xHat / xNorm;
else
    sigmaX = zeros(2, 1, 'like', oldState);
end

dInput = currentAB(1) * sigmaX(1) + currentAB(2) * sigmaX(2);
h2DNext = h2D + h2Gain * (dInput - h2D);
dHat = -psiM * l0 * alpha * (dInput - h2DNext);

% The adaptive-state right-hand sides use the same old q/y/lambda state.
estimationError = phi(1) * xHat(1) + phi(2) * xHat(2) + dHat - regressionY;
correction = -gamma * yState;
qDot = -aGain * (qState - phi * phi.');
yDot = -aGain * (yState - phi * estimationError) + qState * correction;
lambdaDot = vri + correction;

qStateNext = qState + sampleTime * qDot;
yStateNext = yState + sampleTime * yDot;
lambdaHatNext = lambdaHat + sampleTime * lambdaDot;

% Preserve the legacy PLL state slots; FluxEKF now owns angle and speed.
nextState = [ ...
    h2VriNext; ...
    h2INext; ...
    h2RegNext; ...
    h2DNext; ...
    qStateNext(1, 1); ...
    qStateNext(2, 1); ...
    qStateNext(1, 2); ...
    qStateNext(2, 2); ...
    yStateNext; ...
    lambdaHatNext; ...
    oldState(15:18)];

if isfinite(sigmaEpsilon) && sigmaEpsilon > zero ...
        && isfinite(xNorm) && xNorm >= sigmaEpsilon ...
        && isfinite(xHat(1)) && isfinite(xHat(2)) ...
        && all(isfinite(nextState(1:14)))
    activeFluxAlphaWb = xHat(1);
    activeFluxBetaWb = xHat(2);
    status = uint8(1);
else
    status = uint8(2);
end
end
