function [commandDQ_Q15, eHatDQ_Q15, stateNext, status] = ...
    rrcDobFixedPointStep(state, previousAppliedVoltageDQ_Q15, ...
    currentDQ_Q15, rawPiVoltageDQ_Q15, coefficientsDQ_Q27, q_Q29, ...
    reset, ramp_Q31, outputLimit_Q15)
%RRCDOBFIXEDPOINTSTEP Pure-integer RRC-DOB reference step.
%   STATE is int32(8x1). Elements 1:6 are Q5.26 observer states ordered as
%   [xd1; xd2; xd3; xq1; xq2; xq3], element 7 is the previous prewarped
%   q in Q2.29, and element 8 is an initialized flag (0 or 1).
%
%   COEFFICIENTSDQ_Q27 is int32(19x2), with d/q axes in the columns:
%     1:9   A11,A12,A13,A21,A22,A23,A31,A32,A33
%     10:15 B11,B12,B21,B22,B31,B32
%     16:18 C1,C2,C3
%     19    current feedthrough D2
%   All coefficients use signed Q4.27. Voltage/current ports are int16
%   Q1.15, q_Q29 is int32 Q2.29, RESET is uint8, RAMP_Q31 is uint32
%   unsigned Q1.31, and OUTPUTLIMIT_Q15 is nonnegative int16 Q1.15.
%
%   The current-state eHat is emitted before the observer update. On a
%   valid frequency change, x2 is rescaled by (qNew/qOld)^2 and x3 by
%   qNew/qOld before eHat and the update. First activation or a ratio
%   outside [0.5, 2.0] clears both axes and records the new q.
%
%   Status values (uint8):
%     0 valid                 1 explicit reset
%     2 invalid interface     5 arithmetic/narrowing overflow
%     6 correction or command saturated
%     7 first activation or frequency-ratio reset

commandDQ_Q15 = zeros(2, 1, 'int16');
eHatDQ_Q15 = zeros(2, 1, 'int16');
stateNext = zeros(8, 1, 'int32');
status = uint8(2);

if ~(isa(state, 'int32') && isequal(size(state), [8, 1]) && ...
        isa(previousAppliedVoltageDQ_Q15, 'int16') && ...
        isequal(size(previousAppliedVoltageDQ_Q15), [2, 1]) && ...
        isa(currentDQ_Q15, 'int16') && ...
        isequal(size(currentDQ_Q15), [2, 1]) && ...
        isa(rawPiVoltageDQ_Q15, 'int16') && ...
        isequal(size(rawPiVoltageDQ_Q15), [2, 1]) && ...
        isa(coefficientsDQ_Q27, 'int32') && ...
        isequal(size(coefficientsDQ_Q27), [19, 2]) && ...
        isa(q_Q29, 'int32') && isscalar(q_Q29) && ...
        isa(reset, 'uint8') && isscalar(reset) && ...
        isa(ramp_Q31, 'uint32') && isscalar(ramp_Q31) && ...
        isa(outputLimit_Q15, 'int16') && isscalar(outputLimit_Q15))
    return
end

commandDQ_Q15 = rawPiVoltageDQ_Q15;
if reset ~= uint8(0)
    status = uint8(1);
    return
end

unityRamp_Q31 = bitshift(uint32(1), 31);
if q_Q29 <= int32(0) || outputLimit_Q15 < int16(0) || ...
        ramp_Q31 > unityRamp_Q31 || ...
        (state(8) ~= int32(0) && state(8) ~= int32(1))
    status = uint8(2);
    return
end

if state(8) == int32(0)
    stateNext(7) = q_Q29;
    stateNext(8) = int32(1);
    status = uint8(7);
    return
end

previousQ_Q29 = state(7);
if previousQ_Q29 <= int32(0)
    status = uint8(2);
    return
end

twiceCurrentQ = int64(2) * int64(q_Q29);
twicePreviousQ = int64(2) * int64(previousQ_Q29);
if twiceCurrentQ < int64(previousQ_Q29) || ...
        int64(q_Q29) > twicePreviousQ
    stateNext(7) = q_Q29;
    stateNext(8) = int32(1);
    status = uint8(7);
    return
end

workingState = state(1:6);
if q_Q29 ~= previousQ_Q29
    ratioNumerator = bitshift(int64(q_Q29), 30);
    ratioNumerator = ratioNumerator + ...
        idivide(int64(previousQ_Q29), int64(2), 'floor');
    ratio_Q30 = idivide(ratioNumerator, int64(previousQ_Q29), 'floor');
    [workingState, rescaleValid] = localRescaleState( ...
        workingState, ratio_Q30);
    if ~rescaleValid
        status = uint8(5);
        return
    end
end

[eHatDQ_Q15(1), stateNext(1:3), dAxisValid] = localAxisStep( ...
    workingState(1:3), previousAppliedVoltageDQ_Q15(1), ...
    currentDQ_Q15(1), coefficientsDQ_Q27(:, 1));
[eHatDQ_Q15(2), stateNext(4:6), qAxisValid] = localAxisStep( ...
    workingState(4:6), previousAppliedVoltageDQ_Q15(2), ...
    currentDQ_Q15(2), coefficientsDQ_Q27(:, 2));

if ~(dAxisValid && qAxisValid)
    eHatDQ_Q15(:) = int16(0);
    stateNext(:) = int32(0);
    status = uint8(5);
    return
end
stateNext(7) = q_Q29;
stateNext(8) = int32(1);

sampleSaturated = false;
for axisIndex = 1:2
    rampProduct = int64(eHatDQ_Q15(axisIndex)) * int64(ramp_Q31);
    [requestedCorrection, roundingValid] = ...
        localRoundShiftSigned(rampProduct, 31);
    if ~roundingValid
        commandDQ_Q15 = rawPiVoltageDQ_Q15;
        eHatDQ_Q15(:) = int16(0);
        stateNext(:) = int32(0);
        status = uint8(5);
        return
    end

    upperLimit = int64(outputLimit_Q15);
    lowerLimit = -upperLimit;
    limitedCorrection = requestedCorrection;
    if limitedCorrection > upperLimit
        limitedCorrection = upperLimit;
        sampleSaturated = true;
    elseif limitedCorrection < lowerLimit
        limitedCorrection = lowerLimit;
        sampleSaturated = true;
    end

    commandWide = int64(rawPiVoltageDQ_Q15(axisIndex)) - limitedCorrection;
    if commandWide > int64(intmax('int16'))
        commandWide = int64(intmax('int16'));
        sampleSaturated = true;
    elseif commandWide < int64(intmin('int16'))
        commandWide = int64(intmin('int16'));
        sampleSaturated = true;
    end
    commandDQ_Q15(axisIndex) = int16(commandWide);
end

if sampleSaturated
    status = uint8(6);
else
    status = uint8(0);
end
end

function [stateRescaled, valid] = localRescaleState(state, ratio_Q30)
stateRescaled = state;
[stateRescaled(2), dState2FirstValid] = ...
    localScaleStateValue(stateRescaled(2), ratio_Q30);
[stateRescaled(2), dState2SecondValid] = ...
    localScaleStateValue(stateRescaled(2), ratio_Q30);
[stateRescaled(3), dState3Valid] = ...
    localScaleStateValue(stateRescaled(3), ratio_Q30);
[stateRescaled(5), qState2FirstValid] = ...
    localScaleStateValue(stateRescaled(5), ratio_Q30);
[stateRescaled(5), qState2SecondValid] = ...
    localScaleStateValue(stateRescaled(5), ratio_Q30);
[stateRescaled(6), qState3Valid] = ...
    localScaleStateValue(stateRescaled(6), ratio_Q30);
valid = dState2FirstValid && dState2SecondValid && dState3Valid && ...
    qState2FirstValid && qState2SecondValid && qState3Valid;
end

function [scaledValue, valid] = localScaleStateValue(value, ratio_Q30)
product = int64(value) * ratio_Q30;
[roundedValue, roundingValid] = localRoundShiftSigned(product, 30);
valid = roundingValid && roundedValue <= int64(intmax('int32')) && ...
    roundedValue >= int64(intmin('int32'));
if valid
    scaledValue = int32(roundedValue);
else
    scaledValue = int32(0);
end
end

function [eHat_Q15, stateNext, valid] = localAxisStep(state, ...
    previousAppliedVoltage_Q15, current_Q15, coefficients_Q27)
stateNext = zeros(3, 1, 'int32');
eHat_Q15 = int16(0);

for rowIndex = 1:3
    coefficientOffset = (rowIndex - 1) * 3;
    inputOffset = 10 + (rowIndex - 1) * 2;
    accumulator = int64(0);
    [accumulator, valid] = localAccumulate(accumulator, ...
        int64(coefficients_Q27(coefficientOffset + 1)) * int64(state(1)));
    if ~valid
        return
    end
    [accumulator, valid] = localAccumulate(accumulator, ...
        int64(coefficients_Q27(coefficientOffset + 2)) * int64(state(2)));
    if ~valid
        return
    end
    [accumulator, valid] = localAccumulate(accumulator, ...
        int64(coefficients_Q27(coefficientOffset + 3)) * int64(state(3)));
    if ~valid
        return
    end
    voltageTerm = bitshift( ...
        int64(coefficients_Q27(inputOffset)) * ...
        int64(previousAppliedVoltage_Q15), 11);
    [accumulator, valid] = localAccumulate(accumulator, voltageTerm);
    if ~valid
        return
    end
    currentTerm = bitshift( ...
        int64(coefficients_Q27(inputOffset + 1)) * int64(current_Q15), 11);
    [accumulator, valid] = localAccumulate(accumulator, currentTerm);
    if ~valid
        return
    end
    [roundedState, valid] = localRoundShiftSigned(accumulator, 27);
    if ~valid || roundedState > int64(intmax('int32')) || ...
            roundedState < int64(intmin('int32'))
        valid = false;
        return
    end
    stateNext(rowIndex) = int32(roundedState);
end

outputAccumulator = int64(0);
for stateIndex = 1:3
    outputTerm = int64(coefficients_Q27(15 + stateIndex)) * ...
        int64(state(stateIndex));
    [outputAccumulator, valid] = localAccumulate( ...
        outputAccumulator, outputTerm);
    if ~valid
        return
    end
end
feedthroughTerm = bitshift(int64(coefficients_Q27(19)) * ...
    int64(current_Q15), 11);
[outputAccumulator, valid] = localAccumulate( ...
    outputAccumulator, feedthroughTerm);
if ~valid
    return
end
[roundedOutput, valid] = localRoundShiftSigned(outputAccumulator, 38);
if ~valid || roundedOutput > int64(intmax('int16')) || ...
        roundedOutput < int64(intmin('int16'))
    valid = false;
    return
end
eHat_Q15 = int16(roundedOutput);
end

function [result, valid] = localAccumulate(accumulator, term)
maximumValue = intmax('int64');
minimumValue = intmin('int64');
valid = true;
if term > int64(0)
    if accumulator > maximumValue - term
        valid = false;
    end
elseif term < int64(0)
    if accumulator < minimumValue - term
        valid = false;
    end
end
if valid
    result = accumulator + term;
else
    result = int64(0);
end
end

function [roundedValue, valid] = localRoundShiftSigned(value, shiftCount)
halfLsb = bitshift(int64(1), shiftCount - 1);
maximumValue = intmax('int64');
valid = true;
if value >= int64(0)
    if value > maximumValue - halfLsb
        valid = false;
        roundedValue = int64(0);
    else
        roundedValue = bitshift(value + halfLsb, -shiftCount);
    end
elseif value == intmin('int64')
    valid = false;
    roundedValue = int64(0);
else
    magnitude = -value;
    if magnitude > maximumValue - halfLsb
        valid = false;
        roundedValue = int64(0);
    else
        roundedValue = -bitshift(magnitude + halfLsb, -shiftCount);
    end
end
end
