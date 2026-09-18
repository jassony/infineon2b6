function [positionPU, speedPU, fluxMagnitudePU, activeFlux_Wb, ...
    rawOmega_radps, status] = kre_external_observer_diagnostic_step( ...
    kreEnable, viFb, params, pllParams)
%KRE_EXTERNAL_OBSERVER_DIAGNOSTIC_STEP KRE step with raw diagnostics.
%#codegen
%
% The first four outputs preserve the existing wrapper contract. The two
% added outputs expose the pre-update active-flux magnitude and the raw PLL
% electrical speed used by the full-parameter VAFID sidecar. All values are
% produced from the same delayed VI tuple consumed by the KRE state update.

positionPU = single(0.0);
speedPU = single(0.0);
fluxMagnitudePU = single(0.0);
activeFlux_Wb = single(0.0);
rawOmega_radps = single(0.0);
status = uint8(0);

if kreEnable == 0
    return;
end

if ~localInputsValid(viFb, params, pllParams)
    % Preserve the established wrapper invalid-data status. The firmware
    % adapter performs the finer parameter-versus-numerical classification.
    status = uint8(2);
    return;
end

[speedPU, positionPU, activeFlux_Wb, rawOmega_radps] = ...
    localKreObserverStep(viFb, params, pllParams);

if ~isfinite(positionPU) || ~isfinite(speedPU) || ...
        ~isfinite(activeFlux_Wb) || ~isfinite(rawOmega_radps)
    positionPU = single(0.0);
    speedPU = single(0.0);
    activeFlux_Wb = single(0.0);
    rawOmega_radps = single(0.0);
    status = uint8(2);
else
    status = uint8(1);
end
end

function valid = localInputsValid(viFb, params, pllParams)
valid = true;
for index = 1:4
    valid = valid && isfinite(viFb(index));
end
for index = 1:14
    valid = valid && isfinite(params(index));
end
for index = 1:2
    valid = valid && isfinite(pllParams(index));
end

valid = valid && params(1) > single(0.0) ...
    && params(2) > single(0.0) && params(3) > single(0.0) ...
    && params(4) > single(0.0) && params(5) > single(0.0) ...
    && params(6) > single(0.0) && params(7) > single(0.0) ...
    && params(8) > single(0.0) && params(9) > single(0.0) ...
    && params(10) > single(0.0) && params(11) > single(0.0) ...
    && params(12) > single(0.0) && params(13) > single(0.0) ...
    && params(14) >= single(0.0) && pllParams(1) > single(0.0) ...
    && pllParams(2) > single(0.0);
end

function [speedPU, posPU, activeFlux_Wb, rawOmega_radps] = ...
    localKreObserverStep(viFb, params, pllParams)
% Fixed-size discrete KRE active-flux observer and Type-II PLL.

persistent h2Vri h2I h2Reg h2D qState yState lambdaHat ...
    pllTheta pllOmegaInt speedFilt pllInitialized

if isempty(pllInitialized)
    sample = viFb(1);
    h2Vri = zeros(2, 1, 'like', sample);
    h2I = zeros(2, 1, 'like', sample);
    h2Reg = zeros(1, 1, 'like', sample);
    h2D = zeros(1, 1, 'like', sample);
    qState = zeros(2, 2, 'like', sample);
    yState = zeros(2, 1, 'like', sample);
    lambdaHat = zeros(2, 1, 'like', sample);
    pllTheta = zeros(1, 1, 'like', sample);
    pllOmegaInt = zeros(1, 1, 'like', sample);
    speedFilt = zeros(1, 1, 'like', sample);
    pllInitialized = false;
end

resistance = params(1);
ld = params(2);
lq = params(3);
psiM = params(4);
sampleTime = params(5);
voltageBase = params(6);
currentBase = params(7);
speedBase = params(8);
polePairs = params(9);
alpha = params(10);
aGain = params(11);
gamma = params(12);
sigmaEpsilon = params(13);
speedFilterHz = params(14);
pllBandwidthHz = pllParams(1);
pllDamping = pllParams(2);

voltageAB = [viFb(1); viFb(2)] * voltageBase;
currentAB = [viFb(3); viFb(4)] * currentBase;
l0 = ld - lq;

h2Gain = single(1.0) - exp(-alpha * sampleTime);
vri = voltageAB - resistance * currentAB;
h2Vri = h2Vri + h2Gain * (vri - h2Vri);
h2I = h2I + h2Gain * (currentAB - h2I);
h1I = alpha * (currentAB - h2I);

omega1 = h2Vri - lq * h1I;
omega2 = omega1 - l0 * h1I;
phi = omega1 + omega2;

regressionProduct = omega2(1) * omega1(1) ...
    + omega2(2) * omega1(2);
h2Reg = h2Reg + h2Gain * (regressionProduct - h2Reg);
regressionY = l0 * (h2I(1) * omega1(1) ...
    + h2I(2) * omega1(2)) ...
    + (omega1(1) * omega1(1) + omega1(2) * omega1(2)) / alpha ...
    + h2Reg / alpha;

xHat = lambdaHat - lq * currentAB;
activeFlux_Wb = sqrt(xHat(1) * xHat(1) + xHat(2) * xHat(2));
if activeFlux_Wb >= sigmaEpsilon
    sigmaX = xHat / activeFlux_Wb;
else
    sigmaX = zeros(2, 1, 'like', xHat);
end

dInput = currentAB(1) * sigmaX(1) + currentAB(2) * sigmaX(2);
h2D = h2D + h2Gain * (dInput - h2D);
dHat = -psiM * l0 * alpha * (dInput - h2D);

estimationError = phi(1) * xHat(1) + phi(2) * xHat(2) ...
    + dHat - regressionY;
correction = -gamma * yState;
qDot = -aGain * (qState - phi * phi.');
yDot = -aGain * (yState - phi * estimationError) ...
    + qState * correction;
lambdaDot = voltageAB - resistance * currentAB + correction;

qState = qState + sampleTime * qDot;
yState = yState + sampleTime * yDot;
lambdaHat = lambdaHat + sampleTime * lambdaDot;

thetaRaw = atan2(xHat(2), xHat(1));
if activeFlux_Wb >= sigmaEpsilon
    if pllInitialized
        phaseError = localWrapMinusPiToPi(thetaRaw - pllTheta);
        pllNaturalFrequency = single(2.0 * pi) * pllBandwidthHz;
        pllKp = single(2.0) * pllDamping * pllNaturalFrequency;
        pllKi = pllNaturalFrequency * pllNaturalFrequency;
        pllOmegaInt = pllOmegaInt + sampleTime * pllKi * phaseError;
        rawOmega_radps = pllOmegaInt + pllKp * phaseError;
        pllTheta = localWrapMinusPiToPi( ...
            pllTheta + sampleTime * rawOmega_radps);
    else
        pllTheta = thetaRaw;
        pllOmegaInt = zeros(1, 1, 'like', thetaRaw);
        rawOmega_radps = zeros(1, 1, 'like', thetaRaw);
        pllInitialized = true;
    end
else
    rawOmega_radps = pllOmegaInt;
end

if pllInitialized
    if pllTheta < single(0.0)
        posPU = pllTheta / single(2.0 * pi) + single(1.0);
    else
        posPU = pllTheta / single(2.0 * pi);
    end
    speedRaw = rawOmega_radps * single(60.0) ...
        / (single(2.0 * pi) * polePairs * speedBase);
else
    posPU = zeros(1, 1, 'like', thetaRaw);
    speedRaw = zeros(1, 1, 'like', thetaRaw);
end

speedGain = single(1.0) - exp(-single(2.0 * pi) ...
    * speedFilterHz * sampleTime);
speedFilt = speedFilt + speedGain * (speedRaw - speedFilt);
speedPU = speedFilt;
end

function wrapped = localWrapMinusPiToPi(angle)
twoPi = single(2.0 * pi);
piValue = single(pi);
wrapped = angle - floor((angle + piValue) / twoPi) * twoPi;
if wrapped >= piValue
    wrapped = wrapped - twoPi;
elseif wrapped < -piValue
    wrapped = wrapped + twoPi;
end
end
