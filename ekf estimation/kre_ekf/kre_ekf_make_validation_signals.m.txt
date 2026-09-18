function signals = kre_ekf_make_validation_signals(config)
%KRE_EKF_MAKE_VALIDATION_SIGNALS Build PMSM inputs for the KRE-to-EKF path.
%
% KRE receives per-unit alpha-beta voltage/current samples. Its active-flux
% output is normalized and passed to FluxEKF inside the Simulink model.

sampleCount = 2401;
sampleIndex = (0:sampleCount - 1).';
time = sampleIndex * config.Ts;

speedPUTrue = 0.25 * ones(sampleCount, 1);
rampMask = time >= 0.01;
speedPUTrue(rampMask) = 0.8 - 0.55 * exp(-(time(rampMask) - 0.01) / 0.012);

thetaElectricalRadTrue = zeros(sampleCount, 1);
thetaElectricalRadTrue(2:end) = cumsum( ...
    config.beta * speedPUTrue(1:end - 1));
positionPUTrue = mod(thetaElectricalRadTrue / (2 * pi), 1);

% v_ab = R_s i_ab + d(lambda_ab)/dt for an ideal PMSM trajectory.
idA = zeros(sampleCount, 1);
iqA = config.validationIqPU * config.baseCurrentA * ones(sampleCount, 1);
theta = thetaElectricalRadTrue;
currentAlphaA = idA .* cos(theta) - iqA .* sin(theta);
currentBetaA = idA .* sin(theta) + iqA .* cos(theta);

lambdaD = config.LdH .* idA + config.psiMWb;
lambdaQ = config.LqH .* iqA;
lambdaAlphaWb = lambdaD .* cos(theta) - lambdaQ .* sin(theta);
lambdaBetaWb = lambdaD .* sin(theta) + lambdaQ .* cos(theta);
dLambdaAlphaV = local_forward_difference(lambdaAlphaWb, config.Ts);
dLambdaBetaV = local_forward_difference(lambdaBetaWb, config.Ts);
voltageAlphaV = config.RsOhm .* currentAlphaA + dLambdaAlphaV;
voltageBetaV = config.RsOhm .* currentBetaA + dLambdaBetaV;

kreViFbPU = single([voltageAlphaV / config.baseVoltageV, ...
    voltageBetaV / config.baseVoltageV, ...
    currentAlphaA / config.baseCurrentA, ...
    currentBetaA / config.baseCurrentA]);

kreReset = uint8(zeros(sampleCount, 1));
kreReset(1) = uint8(1);

signals = struct();
signals.time = time;
signals.thetaElectricalRadTrue = thetaElectricalRadTrue;
signals.positionPUTrue = positionPUTrue;
signals.speedPUTrue = speedPUTrue;
signals.kreEnable = uint8(ones(sampleCount, 1));
signals.kreViFbPU = kreViFbPU;
signals.kreParams = repmat(config.kreParams(:).', sampleCount, 1);
signals.kreReset = kreReset;
signals.krePllParams = repmat(config.krePllParams(:).', sampleCount, 1);
signals.voltageAlphaV = voltageAlphaV;
signals.voltageBetaV = voltageBetaV;
signals.currentAlphaA = currentAlphaA;
signals.currentBetaA = currentBetaA;
end

function derivative = local_forward_difference(value, sampleTime)
derivative = zeros(size(value));
derivative(1:end - 1) = diff(value) / sampleTime;
derivative(end) = derivative(end - 1);
end
