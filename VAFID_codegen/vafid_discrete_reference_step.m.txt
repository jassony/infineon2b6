function [probeD_PU, probeQ_PU, rs_Ohm, ld_H, lq_H, fluxPM_Wb, ...
    estimateValid, freshEstimate, staleEstimate, status, ...
    conditionNumber, relativeResidual, consecutiveAcceptedWindows, ...
    windowSampleCount] = vafid_discrete_reference_step( ...
    mode, resetRequest, sampleValid, voltageAlpha_V, voltageBeta_V, ...
    currentAlpha_A, currentBeta_A, kreElectricalAngle_rad, ...
    kreElectricalOmega_radps, kreActiveFlux_Wb, config)
%VAFID_DISCRETE_REFERENCE_STEP Fixed-size online IPMSM parameter reference.
%   This function implements the first, observer-only VAFID integration gate.
%   It creates two current-reference probes and identifies [Rs Ld Lq] from
%   two complex steady-state dq equations. FluxPM is recovered from the KRE
%   active-flux signal after the saliency term is removed.
%
%   mode == 0 is deliberately inert: probes are zero, no accumulator or
%   fallback estimator runs, and the estimate is marked stale/invalid.
%
%   status values:
%     0 off, 1 reset, 2 settling, 3 collecting, 4 accepted window,
%     5 ill-conditioned, 6 excessive residual, 7 out of range,
%     8 invalid input/configuration, 10 singular solve.

%#codegen

persistent phaseD_rad phaseQ_rad trackedAngle_rad filteredOmega_radps
persistent filteredActiveFlux_Wb activeFluxInitialized angleInitialized
persistent phasorReal phasorImag sumOmega_radps sumId_A
persistent settleSampleCount accumulatedSampleCount
persistent rsEstimate_Ohm ldEstimate_H lqEstimate_H fluxEstimate_Wb
persistent acceptedWindowCount rejectedWindowCount estimateIsCurrent
persistent wasEnabled lastConditionNumber lastRelativeResidual

% Initialize each persistent independently. MATLAB Coder does not infer that
% checking one persistent proves the remaining persistent values initialized.
if isempty(phaseD_rad)
    phaseD_rad = single(0.0);
end
if isempty(phaseQ_rad)
    phaseQ_rad = single(0.0);
end
if isempty(trackedAngle_rad)
    trackedAngle_rad = single(0.0);
end
if isempty(filteredOmega_radps)
    filteredOmega_radps = single(0.0);
end
if isempty(filteredActiveFlux_Wb)
    filteredActiveFlux_Wb = single(0.0);
end
if isempty(activeFluxInitialized)
    activeFluxInitialized = false;
end
if isempty(angleInitialized)
    angleInitialized = false;
end
if isempty(phasorReal)
    phasorReal = zeros(4, 2, 'single');
end
if isempty(phasorImag)
    phasorImag = zeros(4, 2, 'single');
end
if isempty(sumOmega_radps)
    sumOmega_radps = single(0.0);
end
if isempty(sumId_A)
    sumId_A = single(0.0);
end
if isempty(settleSampleCount)
    settleSampleCount = uint32(0);
end
if isempty(accumulatedSampleCount)
    accumulatedSampleCount = uint32(0);
end
if isempty(rsEstimate_Ohm)
    rsEstimate_Ohm = config.nominalRs_Ohm;
end
if isempty(ldEstimate_H)
    ldEstimate_H = config.nominalLd_H;
end
if isempty(lqEstimate_H)
    lqEstimate_H = config.nominalLq_H;
end
if isempty(fluxEstimate_Wb)
    fluxEstimate_Wb = config.nominalFluxPM_Wb;
end
if isempty(acceptedWindowCount)
    acceptedWindowCount = uint16(0);
end
if isempty(rejectedWindowCount)
    rejectedWindowCount = uint16(0);
end
if isempty(estimateIsCurrent)
    estimateIsCurrent = false;
end
if isempty(wasEnabled)
    wasEnabled = false;
end
if isempty(lastConditionNumber)
    lastConditionNumber = single(Inf);
end
if isempty(lastRelativeResidual)
    lastRelativeResidual = single(Inf);
end

probeD_PU = single(0.0);
probeQ_PU = single(0.0);
freshEstimate = false;

if resetRequest
    phaseD_rad = single(0.0);
    phaseQ_rad = single(0.0);
    trackedAngle_rad = single(0.0);
    filteredOmega_radps = single(0.0);
    filteredActiveFlux_Wb = single(0.0);
    activeFluxInitialized = false;
    angleInitialized = false;
    phasorReal(:) = single(0.0);
    phasorImag(:) = single(0.0);
    sumOmega_radps = single(0.0);
    sumId_A = single(0.0);
    settleSampleCount = uint32(0);
    accumulatedSampleCount = uint32(0);
    rsEstimate_Ohm = config.nominalRs_Ohm;
    ldEstimate_H = config.nominalLd_H;
    lqEstimate_H = config.nominalLq_H;
    fluxEstimate_Wb = config.nominalFluxPM_Wb;
    acceptedWindowCount = uint16(0);
    rejectedWindowCount = uint16(0);
    estimateIsCurrent = false;
    wasEnabled = false;
    lastConditionNumber = single(Inf);
    lastRelativeResidual = single(Inf);
    status = uint8(1);
    [rs_Ohm, ld_H, lq_H, fluxPM_Wb, estimateValid, staleEstimate, ...
        conditionNumber, relativeResidual, consecutiveAcceptedWindows, ...
        windowSampleCount] = localOutputs(rsEstimate_Ohm, ldEstimate_H, ...
        lqEstimate_H, fluxEstimate_Wb, estimateIsCurrent, ...
        lastConditionNumber, lastRelativeResidual, acceptedWindowCount, ...
        accumulatedSampleCount);
    return;
end

if mode == uint8(0)
    phaseD_rad = single(0.0);
    phaseQ_rad = single(0.0);
    phasorReal(:) = single(0.0);
    phasorImag(:) = single(0.0);
    sumOmega_radps = single(0.0);
    sumId_A = single(0.0);
    settleSampleCount = uint32(0);
    accumulatedSampleCount = uint32(0);
    acceptedWindowCount = uint16(0);
    rejectedWindowCount = uint16(0);
    estimateIsCurrent = false;
    wasEnabled = false;
    angleInitialized = false;
    activeFluxInitialized = false;
    status = uint8(0);
    [rs_Ohm, ld_H, lq_H, fluxPM_Wb, estimateValid, staleEstimate, ...
        conditionNumber, relativeResidual, consecutiveAcceptedWindows, ...
        windowSampleCount] = localOutputs(rsEstimate_Ohm, ldEstimate_H, ...
        lqEstimate_H, fluxEstimate_Wb, estimateIsCurrent, ...
        lastConditionNumber, lastRelativeResidual, acceptedWindowCount, ...
        accumulatedSampleCount);
    return;
end

configurationValid = localConfigurationIsValid(config);
finiteInputs = isfinite(voltageAlpha_V) && isfinite(voltageBeta_V) && ...
    isfinite(currentAlpha_A) && isfinite(currentBeta_A) && ...
    isfinite(kreElectricalAngle_rad) && ...
    isfinite(kreElectricalOmega_radps) && isfinite(kreActiveFlux_Wb);
if ~configurationValid || ~sampleValid || ~finiteInputs
    probeD_PU = single(0.0);
    probeQ_PU = single(0.0);
    phaseD_rad = single(0.0);
    phaseQ_rad = single(0.0);
    phasorReal(:) = single(0.0);
    phasorImag(:) = single(0.0);
    sumOmega_radps = single(0.0);
    sumId_A = single(0.0);
    settleSampleCount = uint32(0);
    accumulatedSampleCount = uint32(0);
    acceptedWindowCount = uint16(0);
    rejectedWindowCount = uint16(0);
    estimateIsCurrent = false;
    wasEnabled = false;
    angleInitialized = false;
    activeFluxInitialized = false;
    status = uint8(8);
    [rs_Ohm, ld_H, lq_H, fluxPM_Wb, estimateValid, staleEstimate, ...
        conditionNumber, relativeResidual, consecutiveAcceptedWindows, ...
        windowSampleCount] = localOutputs(rsEstimate_Ohm, ldEstimate_H, ...
        lqEstimate_H, fluxEstimate_Wb, estimateIsCurrent, ...
        lastConditionNumber, lastRelativeResidual, acceptedWindowCount, ...
        accumulatedSampleCount);
    return;
end

if ~wasEnabled
    phaseD_rad = single(0.0);
    phaseQ_rad = single(0.0);
    trackedAngle_rad = localWrapAngle(kreElectricalAngle_rad);
    filteredOmega_radps = kreElectricalOmega_radps;
    filteredActiveFlux_Wb = kreActiveFlux_Wb;
    activeFluxInitialized = true;
    angleInitialized = true;
    phasorReal(:) = single(0.0);
    phasorImag(:) = single(0.0);
    sumOmega_radps = single(0.0);
    sumId_A = single(0.0);
    settleSampleCount = uint32(0);
    accumulatedSampleCount = uint32(0);
    acceptedWindowCount = uint16(0);
    rejectedWindowCount = uint16(0);
    estimateIsCurrent = false;
    wasEnabled = true;
end

probeDAmplitude_PU = min(abs(config.probeD_Amplitude_PU), ...
    config.probeD_MaxAmplitude_PU);
probeQAmplitude_PU = min(abs(config.probeQ_Amplitude_PU), ...
    config.probeQ_MaxAmplitude_PU);
probeD_PU = probeDAmplitude_PU * sin(phaseD_rad);
probeQ_PU = probeQAmplitude_PU * sin(phaseQ_rad);

omegaAlpha = localOnePoleAlpha(config.omegaLpf_Hz, config.sampleTime_s);
angleAlpha = localOnePoleAlpha(config.angleTrack_Hz, config.sampleTime_s);
fluxAlpha = localOnePoleAlpha(config.activeFluxLpf_Hz, config.sampleTime_s);
filteredOmega_radps = filteredOmega_radps + ...
    omegaAlpha * (kreElectricalOmega_radps - filteredOmega_radps);
if ~angleInitialized
    trackedAngle_rad = localWrapAngle(kreElectricalAngle_rad);
    angleInitialized = true;
else
    predictedAngle_rad = localWrapAngle(trackedAngle_rad + ...
        config.sampleTime_s * filteredOmega_radps);
    angleError_rad = localWrapAngle(kreElectricalAngle_rad - predictedAngle_rad);
    trackedAngle_rad = localWrapAngle(predictedAngle_rad + ...
        angleAlpha * angleError_rad);
end
if ~activeFluxInitialized
    filteredActiveFlux_Wb = kreActiveFlux_Wb;
    activeFluxInitialized = true;
else
    filteredActiveFlux_Wb = filteredActiveFlux_Wb + ...
        fluxAlpha * (kreActiveFlux_Wb - filteredActiveFlux_Wb);
end

cosAngle = cos(trackedAngle_rad);
sinAngle = sin(trackedAngle_rad);
voltageD_V = cosAngle * voltageAlpha_V + sinAngle * voltageBeta_V;
voltageQ_V = -sinAngle * voltageAlpha_V + cosAngle * voltageBeta_V;
currentD_A = cosAngle * currentAlpha_A + sinAngle * currentBeta_A;
currentQ_A = -sinAngle * currentAlpha_A + cosAngle * currentBeta_A;

settleSamples = uint32(round(config.settleTime_s / config.sampleTime_s));
if settleSampleCount < settleSamples
    settleSampleCount = settleSampleCount + uint32(1);
    status = uint8(2);
else
    signals = [voltageD_V; voltageQ_V; currentD_A; currentQ_A];
    phaseCosine = [cos(phaseD_rad), cos(phaseQ_rad)];
    phaseSine = [sin(phaseD_rad), sin(phaseQ_rad)];
    for frequencyIndex = 1:2
        phasorReal(:, frequencyIndex) = phasorReal(:, frequencyIndex) + ...
            signals * phaseCosine(frequencyIndex);
        phasorImag(:, frequencyIndex) = phasorImag(:, frequencyIndex) - ...
            signals * phaseSine(frequencyIndex);
    end
    sumOmega_radps = sumOmega_radps + filteredOmega_radps;
    sumId_A = sumId_A + currentD_A;
    accumulatedSampleCount = accumulatedSampleCount + uint32(1);
    status = uint8(3);

    if accumulatedSampleCount >= config.windowLength_samples
        phasorScale = single(2.0) / single(config.windowLength_samples);
        voltageDPhasor = complex( ...
            phasorScale * phasorReal(1, :), ...
            phasorScale * phasorImag(1, :));
        voltageQPhasor = complex( ...
            phasorScale * phasorReal(2, :), ...
            phasorScale * phasorImag(2, :));
        currentDPhasor = complex( ...
            phasorScale * phasorReal(3, :), ...
            phasorScale * phasorImag(3, :));
        currentQPhasor = complex( ...
            phasorScale * phasorReal(4, :), ...
            phasorScale * phasorImag(4, :));
        meanOmega_radps = sumOmega_radps / single(config.windowLength_samples);
        meanId_A = sumId_A / single(config.windowLength_samples);
        [candidate, solved, candidateCondition, candidateResidual] = ...
            localSolveParameters(voltageDPhasor, voltageQPhasor, ...
            currentDPhasor, currentQPhasor, meanOmega_radps, config);
        lastConditionNumber = candidateCondition;
        lastRelativeResidual = candidateResidual;

        parameterBoundsValid = solved && ...
            candidate(1) >= config.minimumRs_Ohm && ...
            candidate(1) <= config.maximumRs_Ohm && ...
            candidate(2) >= config.minimumLd_H && ...
            candidate(2) <= config.maximumLd_H && ...
            candidate(3) >= config.minimumLq_H && ...
            candidate(3) <= config.maximumLq_H;
        candidateFlux_Wb = filteredActiveFlux_Wb - ...
            (candidate(2) - candidate(3)) * meanId_A;
        fluxBoundsValid = isfinite(candidateFlux_Wb) && ...
            candidateFlux_Wb >= config.minimumFluxPM_Wb && ...
            candidateFlux_Wb <= config.maximumFluxPM_Wb;

        if ~solved
            status = uint8(10);
            acceptedWindowCount = uint16(0);
            rejectedWindowCount = localIncrementCounter(rejectedWindowCount);
        elseif candidateCondition >= config.conditionLimit
            status = uint8(5);
            acceptedWindowCount = uint16(0);
            rejectedWindowCount = localIncrementCounter(rejectedWindowCount);
        elseif candidateResidual >= config.relativeResidualLimit
            status = uint8(6);
            acceptedWindowCount = uint16(0);
            rejectedWindowCount = localIncrementCounter(rejectedWindowCount);
        elseif ~parameterBoundsValid || ~fluxBoundsValid
            status = uint8(7);
            acceptedWindowCount = uint16(0);
            rejectedWindowCount = localIncrementCounter(rejectedWindowCount);
        else
            rsEstimate_Ohm = rsEstimate_Ohm + config.parameterFusion * ...
                (candidate(1) - rsEstimate_Ohm);
            ldEstimate_H = ldEstimate_H + config.parameterFusion * ...
                (candidate(2) - ldEstimate_H);
            lqEstimate_H = lqEstimate_H + config.parameterFusion * ...
                (candidate(3) - lqEstimate_H);
            acceptedWindowCount = localIncrementCounter(acceptedWindowCount);
            rejectedWindowCount = uint16(0);
            status = uint8(4);
            freshEstimate = true;
            if acceptedWindowCount >= config.requiredAcceptedWindows
                correctedFlux_Wb = filteredActiveFlux_Wb - ...
                    (ldEstimate_H - lqEstimate_H) * meanId_A;
                fluxEstimate_Wb = fluxEstimate_Wb + config.fluxFusion * ...
                    (correctedFlux_Wb - fluxEstimate_Wb);
                estimateIsCurrent = true;
            else
                estimateIsCurrent = false;
            end
        end

        if rejectedWindowCount >= config.staleRejectedWindows
            estimateIsCurrent = false;
        end
        phasorReal(:) = single(0.0);
        phasorImag(:) = single(0.0);
        sumOmega_radps = single(0.0);
        sumId_A = single(0.0);
        accumulatedSampleCount = uint32(0);
    end
end

phaseD_rad = localWrapAngle(phaseD_rad + single(2.0 * pi) * ...
    config.probeD_Frequency_Hz * config.sampleTime_s);
phaseQ_rad = localWrapAngle(phaseQ_rad + single(2.0 * pi) * ...
    config.probeQ_Frequency_Hz * config.sampleTime_s);

[rs_Ohm, ld_H, lq_H, fluxPM_Wb, estimateValid, staleEstimate, ...
    conditionNumber, relativeResidual, consecutiveAcceptedWindows, ...
    windowSampleCount] = localOutputs(rsEstimate_Ohm, ldEstimate_H, ...
    lqEstimate_H, fluxEstimate_Wb, estimateIsCurrent, ...
    lastConditionNumber, lastRelativeResidual, acceptedWindowCount, ...
    accumulatedSampleCount);
end

function valid = localConfigurationIsValid(config)
valid = isfinite(config.sampleTime_s) && config.sampleTime_s > single(0.0) && ...
    isfinite(config.probeD_Frequency_Hz) && config.probeD_Frequency_Hz > single(0.0) && ...
    isfinite(config.probeQ_Frequency_Hz) && config.probeQ_Frequency_Hz > single(0.0) && ...
    config.probeD_Frequency_Hz ~= config.probeQ_Frequency_Hz && ...
    isfinite(config.probeD_Amplitude_PU) && ...
    isfinite(config.probeQ_Amplitude_PU) && ...
    isfinite(config.probeD_MaxAmplitude_PU) && config.probeD_MaxAmplitude_PU > single(0.0) && ...
    isfinite(config.probeQ_MaxAmplitude_PU) && config.probeQ_MaxAmplitude_PU > single(0.0) && ...
    config.windowLength_samples > uint32(0) && ...
    config.requiredAcceptedWindows > uint16(0) && ...
    config.staleRejectedWindows > uint16(0) && ...
    isfinite(config.parameterFusion) && config.parameterFusion > single(0.0) && ...
    config.parameterFusion <= single(1.0) && ...
    isfinite(config.fluxFusion) && config.fluxFusion > single(0.0) && ...
    config.fluxFusion <= single(1.0) && ...
    isfinite(config.conditionLimit) && config.conditionLimit > single(1.0) && ...
    isfinite(config.relativeResidualLimit) && config.relativeResidualLimit > single(0.0) && ...
    config.minimumRs_Ohm > single(0.0) && config.maximumRs_Ohm > config.minimumRs_Ohm && ...
    config.minimumLd_H > single(0.0) && config.maximumLd_H > config.minimumLd_H && ...
    config.minimumLq_H > single(0.0) && config.maximumLq_H > config.minimumLq_H && ...
    config.minimumFluxPM_Wb > single(0.0) && ...
    config.maximumFluxPM_Wb > config.minimumFluxPM_Wb;
end

function alpha = localOnePoleAlpha(cutoff_Hz, sampleTime_s)
alpha = single(1.0) - exp(-single(2.0 * pi) * cutoff_Hz * sampleTime_s);
end

function angle_rad = localWrapAngle(angle_rad)
angle_rad = mod(angle_rad + single(pi), single(2.0 * pi)) - single(pi);
end

function value = localIncrementCounter(value)
if value < intmax('uint16')
    value = value + uint16(1);
end
end

function [candidate, solved, conditionNumber, relativeResidual] = ...
    localSolveParameters(voltageD, voltageQ, currentD, currentQ, ...
    meanElectricalOmega_radps, config)
candidate = zeros(3, 1, 'single');
regression = zeros(8, 3, 'single');
observation = zeros(8, 1, 'single');
probeOmega_radps = single(2.0 * pi) * ...
    [config.probeD_Frequency_Hz; config.probeQ_Frequency_Hz];

for frequencyIndex = 1:2
    row = (frequencyIndex - 1) * 4;
    idReal = real(currentD(frequencyIndex));
    idImag = imag(currentD(frequencyIndex));
    iqReal = real(currentQ(frequencyIndex));
    iqImag = imag(currentQ(frequencyIndex));
    excitationOmega_radps = probeOmega_radps(frequencyIndex);

    regression(row + 1, :) = [idReal, ...
        -excitationOmega_radps * idImag, ...
        -meanElectricalOmega_radps * iqReal];
    regression(row + 2, :) = [idImag, ...
        excitationOmega_radps * idReal, ...
        -meanElectricalOmega_radps * iqImag];
    regression(row + 3, :) = [iqReal, ...
        meanElectricalOmega_radps * idReal, ...
        -excitationOmega_radps * iqImag];
    regression(row + 4, :) = [iqImag, ...
        meanElectricalOmega_radps * idImag, ...
        excitationOmega_radps * iqReal];

    observation(row + 1) = real(voltageD(frequencyIndex));
    observation(row + 2) = imag(voltageD(frequencyIndex));
    observation(row + 3) = real(voltageQ(frequencyIndex));
    observation(row + 4) = imag(voltageQ(frequencyIndex));
end

columnScale = zeros(3, 1, 'single');
scaledRegression = zeros(8, 3, 'single');
for columnIndex = 1:3
    columnScale(columnIndex) = max(abs(regression(:, columnIndex)));
    if columnScale(columnIndex) > single(1.0e-12)
        scaledRegression(:, columnIndex) = ...
            regression(:, columnIndex) / columnScale(columnIndex);
    end
end

normalMatrix = scaledRegression.' * scaledRegression;
normalRightHandSide = scaledRegression.' * observation;
[inverseNormalMatrix, invertible] = localInverse3x3(normalMatrix);
if ~invertible || any(columnScale <= single(1.0e-12))
    solved = false;
    conditionNumber = single(Inf);
    relativeResidual = single(Inf);
    return;
end

normalCondition = localNormOne(normalMatrix) * localNormOne(inverseNormalMatrix);
conditionNumber = sqrt(max(normalCondition, single(0.0)));
scaledCandidate = inverseNormalMatrix * normalRightHandSide;
candidate = scaledCandidate ./ columnScale;
residual = regression * candidate - observation;
observationNorm = sqrt(sum(observation .* observation));
relativeResidual = sqrt(sum(residual .* residual)) / ...
    max(observationNorm, single(1.0e-12));
solved = all(isfinite(candidate)) && isfinite(conditionNumber) && ...
    isfinite(relativeResidual);
end

function [inverseMatrix, invertible] = localInverse3x3(matrix)
inverseMatrix = zeros(3, 3, 'single');
c11 = matrix(2, 2) * matrix(3, 3) - matrix(2, 3) * matrix(3, 2);
c12 = matrix(2, 3) * matrix(3, 1) - matrix(2, 1) * matrix(3, 3);
c13 = matrix(2, 1) * matrix(3, 2) - matrix(2, 2) * matrix(3, 1);
c21 = matrix(1, 3) * matrix(3, 2) - matrix(1, 2) * matrix(3, 3);
c22 = matrix(1, 1) * matrix(3, 3) - matrix(1, 3) * matrix(3, 1);
c23 = matrix(1, 2) * matrix(3, 1) - matrix(1, 1) * matrix(3, 2);
c31 = matrix(1, 2) * matrix(2, 3) - matrix(1, 3) * matrix(2, 2);
c32 = matrix(1, 3) * matrix(2, 1) - matrix(1, 1) * matrix(2, 3);
c33 = matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1);
determinant = matrix(1, 1) * c11 + matrix(1, 2) * c12 + matrix(1, 3) * c13;
invertible = isfinite(determinant) && abs(determinant) > single(1.0e-9);
if invertible
    inverseMatrix = [c11, c21, c31; c12, c22, c32; c13, c23, c33] / determinant;
end
end

function value = localNormOne(matrix)
value = max(sum(abs(matrix), 1));
end

function [rs_Ohm, ld_H, lq_H, fluxPM_Wb, estimateValid, staleEstimate, ...
    conditionNumber, relativeResidual, consecutiveAcceptedWindows, ...
    windowSampleCount] = localOutputs(rsEstimate_Ohm, ldEstimate_H, ...
    lqEstimate_H, fluxEstimate_Wb, estimateIsCurrent, ...
    lastConditionNumber, lastRelativeResidual, acceptedWindowCount, ...
    accumulatedSampleCount)
rs_Ohm = rsEstimate_Ohm;
ld_H = ldEstimate_H;
lq_H = lqEstimate_H;
fluxPM_Wb = fluxEstimate_Wb;
estimateValid = estimateIsCurrent;
staleEstimate = ~estimateIsCurrent;
conditionNumber = lastConditionNumber;
relativeResidual = lastRelativeResidual;
consecutiveAcceptedWindows = acceptedWindowCount;
windowSampleCount = accumulatedSampleCount;
end
