classdef rrcDobDiscreteStepTest < matlab.unittest.TestCase
    %RRCDOBDISCRETESTEPTEST Unit tests for the single RRC-DOB reference.

    properties (TestParameter)
        operatingPoint = struct( ...
            'rpm800', struct('electricalFrequencyHz', ...
                single(800 * 4 / 60)), ...
            'rpm4000', struct('electricalFrequencyHz', ...
                single(4000 * 4 / 60)))
    end

    methods (Test)
        function testClosedFormMatchesDirectTustin(testCase, operatingPoint)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            parameters(7) = operatingPoint.electricalFrequencyHz;

            [actualD, actualQ, outputD, outputQ, statuses] = ...
                rrcDobDiscreteStepTest.identifyPublicMatrices(parameters);
            [expectedD, expectedOutputD] = ...
                rrcDobDiscreteStepTest.directTustinOracle( ...
                parameters, parameters(2));
            [expectedQ, expectedOutputQ] = ...
                rrcDobDiscreteStepTest.directTustinOracle( ...
                parameters, parameters(3));

            testCase.verifyClass(actualD, 'single');
            testCase.verifyClass(actualQ, 'single');
            testCase.verifyEqual(double(actualD), expectedD, 'AbsTol', 3e-6);
            testCase.verifyEqual(double(actualQ), expectedQ, 'AbsTol', 3e-6);
            testCase.verifyEqual(double(outputD), expectedOutputD, ...
                'AbsTol', 3e-6);
            testCase.verifyEqual(double(outputQ), expectedOutputQ, ...
                'AbsTol', 3e-6);
            testCase.verifyEqual(statuses, zeros(10, 1, 'single'), ...
                'AbsTol', single(0));
        end

        function testEstimateIsPublishedBeforeStateUpdate(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            state = rrcDobDiscreteStepTest.initializedState(parameters);
            previousApplied = single([0.25; -0.20]);
            current = zeros(2, 1, 'single');
            rawPi = single([0.10; -0.10]);
            control = single([0; 1; 0.10]);

            [firstCommand, firstEstimate, stateNext, firstStatus] = ...
                rrcDobDiscreteReferenceStep(state, previousApplied, ...
                current, rawPi, parameters, control);
            [~, secondEstimate, ~, secondStatus] = ...
                rrcDobDiscreteReferenceStep(stateNext, previousApplied, ...
                current, rawPi, parameters, control);

            testCase.verifyEqual(firstEstimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(firstCommand, rawPi, ...
                'AbsTol', single(0));
            testCase.verifyNotEqual(stateNext(1:6), zeros(6, 1, 'single'));
            testCase.verifyGreaterThan(max(abs(secondEstimate)), single(0));
            testCase.verifyEqual([firstStatus; secondStatus], ...
                zeros(2, 1, 'single'), 'AbsTol', single(0));
        end

        function testFirstActivationPublishesZeroCompensationBeat(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            state = single([1; 2; 3; 4; 5; 6; 0; 0]);
            rawPi = single([0.20; -0.15]);
            expectedQ = rrcDobDiscreteStepTest.prewarpedQ(parameters);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(state, single([0.1; -0.1]), ...
                single([0.2; -0.2]), rawPi, parameters, ...
                single([0; 1; 0.1]));

            testCase.verifyEqual(command, rawPi, 'AbsTol', single(0));
            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext(1:6), zeros(6, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext(7:8), ...
                single([expectedQ; 1]), 'AbsTol', single(0));
            testCase.verifyEqual(status, single(7), 'AbsTol', single(0));
        end

        function testFrequencyChangeRescalesBeforeOutputAndUpdate(testCase)
            oldParameters = rrcDobDiscreteStepTest.defaultParameters();
            newParameters = oldParameters;
            newParameters(7) = single(150);
            oldQ = rrcDobDiscreteStepTest.prewarpedQ(oldParameters);
            newQ = rrcDobDiscreteStepTest.prewarpedQ(newParameters);
            ratio = newQ / oldQ;
            state = single([0.10; 0.20; -0.30; -0.10; -0.20; 0.30; ...
                oldQ; 1]);
            rescaledState = state(1:6);
            rescaledState([2, 5]) = rescaledState([2, 5]) * ratio * ratio;
            rescaledState([3, 6]) = rescaledState([3, 6]) * ratio;
            previousApplied = single([0.08; -0.06]);
            current = single([0.04; -0.03]);
            rawPi = single([0.12; -0.11]);
            [matrixD, outputD] = rrcDobDiscreteStepTest.directTustinOracle( ...
                newParameters, newParameters(2));
            [matrixQ, outputQ] = rrcDobDiscreteStepTest.directTustinOracle( ...
                newParameters, newParameters(3));
            inputD = double([rescaledState(1:3); previousApplied(1); current(1)]);
            inputQ = double([rescaledState(4:6); previousApplied(2); current(2)]);
            expectedState = [matrixD * inputD; matrixQ * inputQ];
            expectedEstimate = [outputD * inputD; outputQ * inputQ];

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(state, previousApplied, current, ...
                rawPi, newParameters, single([0; 0; 1]));

            testCase.verifyEqual(double(stateNext(1:6)), expectedState, ...
                'AbsTol', 4e-6);
            testCase.verifyEqual(double(estimate), expectedEstimate, ...
                'AbsTol', 4e-6);
            testCase.verifyEqual(stateNext(7:8), single([newQ; 1]), ...
                'AbsTol', single(0));
            testCase.verifyEqual(command, rawPi, 'AbsTol', single(0));
            testCase.verifyEqual(status, single(0), 'AbsTol', single(0));
        end

        function testFrequencyRatioResetPublishesZeroCompensationBeat(testCase)
            oldParameters = rrcDobDiscreteStepTest.defaultParameters();
            newParameters = oldParameters;
            newParameters(7) = single(400);
            oldQ = rrcDobDiscreteStepTest.prewarpedQ(oldParameters);
            newQ = rrcDobDiscreteStepTest.prewarpedQ(newParameters);
            state = single([1; 2; 3; 4; 5; 6; oldQ; 1]);
            rawPi = single([0.18; -0.16]);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(state, single([0.1; -0.1]), ...
                single([0.2; -0.2]), rawPi, newParameters, ...
                single([0; 1; 0.1]));

            testCase.verifyEqual(command, rawPi, 'AbsTol', single(0));
            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext(1:6), zeros(6, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext(7:8), single([newQ; 1]), ...
                'AbsTol', single(0));
            testCase.verifyEqual(status, single(7), 'AbsTol', single(0));
        end

        function testResetClearsStateAndBypassesCompensation(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            state = single((1:8).');
            previousApplied = single([0.2; -0.1]);
            current = single([0.1; -0.2]);
            rawPi = single([0.3; -0.4]);
            control = single([1; 0.7; 0.1]);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(state, previousApplied, ...
                current, rawPi, parameters, control);

            testCase.verifyEqual(command, rawPi, 'AbsTol', single(0));
            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext, zeros(8, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(status, single(1), 'AbsTol', single(0));
        end

        function testInvalidParameterClearsState(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            parameters(3) = single(0);
            rawPi = single([0.2; -0.1]);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(ones(8, 1, 'single'), ...
                zeros(2, 1, 'single'), zeros(2, 1, 'single'), rawPi, ...
                parameters, single([0; 1; 0.1]));

            testCase.verifyEqual(command, rawPi, 'AbsTol', single(0));
            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext, zeros(8, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(status, single(2), 'AbsTol', single(0));
        end

        function testNonfiniteInputClearsState(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            current = single([NaN; 0.1]);
            rawPi = single([0.2; -0.1]);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(ones(8, 1, 'single'), ...
                zeros(2, 1, 'single'), current, rawPi, parameters, ...
                single([0; 1; 0.1]));

            testCase.verifyEqual(command, rawPi, 'AbsTol', single(0));
            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext, zeros(8, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(status, single(5), 'AbsTol', single(0));
        end

        function testFrequencyOutsideLookupRangeIsRejected(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            parameters(7) = single(5);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(ones(8, 1, 'single'), ...
                zeros(2, 1, 'single'), zeros(2, 1, 'single'), ...
                single([0.2; -0.1]), parameters, single([0; 1; 0.1]));

            testCase.verifyEqual(command, single([0.2; -0.1]), ...
                'AbsTol', single(0));
            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext, zeros(8, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(status, single(3), 'AbsTol', single(0));
        end

        function testNyquistGuardIsDistinctFromFrequencyRange(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            parameters(7) = single(2000);

            [~, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(ones(8, 1, 'single'), ...
                zeros(2, 1, 'single'), zeros(2, 1, 'single'), ...
                zeros(2, 1, 'single'), parameters, single([0; 1; 0.1]));

            testCase.verifyEqual(estimate, zeros(2, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateNext, zeros(8, 1, 'single'), ...
                'AbsTol', single(0));
            testCase.verifyEqual(status, single(4), 'AbsTol', single(0));
        end

        function testCorrectionLimitIsReported(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            state = rrcDobDiscreteStepTest.initializedState(parameters);
            state(1:6) = single([10; 0; 0; -10; 0; 0]);
            rawPi = single([0.2; -0.2]);
            outputLimit = single(0.01);

            [command, estimate, stateNext, status] = ...
                rrcDobDiscreteReferenceStep(state, ...
                zeros(2, 1, 'single'), zeros(2, 1, 'single'), rawPi, ...
                parameters, single([0; 1; outputLimit]));

            testCase.verifyGreaterThan(abs(estimate), ...
                outputLimit * ones(2, 1, 'single'));
            testCase.verifyEqual(abs(rawPi - command), ...
                outputLimit * ones(2, 1, 'single'), ...
                'AbsTol', single(2e-7));
            testCase.verifyTrue(all(isfinite(stateNext)));
            testCase.verifyEqual(status, single(6), 'AbsTol', single(0));
        end

        function testFrequencySweepRemainsFinite(testCase)
            [allFinite, statuses] = ...
                rrcDobDiscreteStepTest.runFrequencySweep();

            testCase.verifyTrue(allFinite);
            testCase.verifyTrue(all(statuses == single(0) | ...
                statuses == single(6) | statuses == single(7)));
            testCase.verifyEqual(nnz(statuses == single(7)), 1);
        end

        function testApsfsmRateIqStepsDoNotShareObserverState(testCase)
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            state = rrcDobDiscreteStepTest.initializedState(parameters);
            state(1:6) = single([0.03; -0.02; 0.01; -0.04; 0.01; 0.02]);
            previousApplied = single([0.05; -0.03]);
            current = single([0.02; 0.08]);
            rawPiBeforeStep = single([0.01; 0.02]);
            rawPiAfterStep = single([0.01; 0.12]);
            control = single([0; 0.5; 0.1]);

            [commandBefore, estimateBefore, stateBefore, statusBefore] = ...
                rrcDobDiscreteReferenceStep(state, previousApplied, ...
                current, rawPiBeforeStep, parameters, control);
            [commandAfter, estimateAfter, stateAfter, statusAfter] = ...
                rrcDobDiscreteReferenceStep(state, previousApplied, ...
                current, rawPiAfterStep, parameters, control);
            [allFinite, replayStatuses] = ...
                rrcDobDiscreteStepTest.runApsfsmLikeReplay();

            testCase.verifyEqual(estimateAfter, estimateBefore, ...
                'AbsTol', single(0));
            testCase.verifyEqual(stateAfter, stateBefore, ...
                'AbsTol', single(0));
            testCase.verifyEqual(commandAfter - commandBefore, ...
                rawPiAfterStep - rawPiBeforeStep, ...
                'AbsTol', single(2e-7));
            testCase.verifyEqual([statusBefore; statusAfter], ...
                zeros(2, 1, 'single'), 'AbsTol', single(0));
            testCase.verifyTrue(allFinite);
            testCase.verifyTrue(all(replayStatuses == single(0) | ...
                replayStatuses == single(6) | replayStatuses == single(7)));
            testCase.verifyEqual(nnz(replayStatuses == single(7)), 1);
        end
    end

    methods (Static, Access = private)
        function parameters = defaultParameters()
            parameters = single([0.5; 1.30e-3; 1.38e-3; 1000; 50; ...
                50e-6; 100; 0.10]);
        end

        function q = prewarpedQ(parameters)
            sixthHarmonic_radps = single(2) * single(pi) * single(6) * ...
                parameters(7);
            q = tan(sixthHarmonic_radps * single(0.5) * parameters(6));
        end

        function state = initializedState(parameters)
            state = zeros(8, 1, 'single');
            state(7) = rrcDobDiscreteStepTest.prewarpedQ(parameters);
            state(8) = single(1);
        end

        function [actualD, actualQ, outputD, outputQ, statuses] = ...
                identifyPublicMatrices(parameters)
            actualD = zeros(3, 5, 'single');
            actualQ = zeros(3, 5, 'single');
            outputD = zeros(1, 5, 'single');
            outputQ = zeros(1, 5, 'single');
            statuses = zeros(10, 1, 'single');
            control = single([0; 0; 1]);
            q = rrcDobDiscreteStepTest.prewarpedQ(parameters);

            for basisIndex = 1:5
                [state, applied, current] = ...
                    rrcDobDiscreteStepTest.axisBasis(basisIndex, true, q);
                [~, estimate, stateNext, statuses(basisIndex)] = ...
                    rrcDobDiscreteReferenceStep(state, applied, current, ...
                    zeros(2, 1, 'single'), parameters, control);
                actualD(:, basisIndex) = stateNext(1:3);
                outputD(basisIndex) = estimate(1);

                [state, applied, current] = ...
                    rrcDobDiscreteStepTest.axisBasis(basisIndex, false, q);
                [~, estimate, stateNext, statuses(5 + basisIndex)] = ...
                    rrcDobDiscreteReferenceStep(state, applied, current, ...
                    zeros(2, 1, 'single'), parameters, control);
                actualQ(:, basisIndex) = stateNext(4:6);
                outputQ(basisIndex) = estimate(2);
            end
        end

        function [state, applied, current] = axisBasis( ...
                basisIndex, useDAxis, q)
            state = zeros(8, 1, 'single');
            state(7:8) = single([q; 1]);
            applied = zeros(2, 1, 'single');
            current = zeros(2, 1, 'single');
            axisOffset = 0;
            axisIndex = 1;
            if ~useDAxis
                axisOffset = 3;
                axisIndex = 2;
            end
            if basisIndex <= 3
                state(axisOffset + basisIndex) = single(1);
            elseif basisIndex == 4
                applied(axisIndex) = single(1);
            else
                current(axisIndex) = single(1);
            end
        end

        function [augmentedMatrix, outputRow] = ...
                directTustinOracle(parameters, inductance_H)
            resistance_Ohm = double(parameters(1));
            inductance_H = double(inductance_H);
            voltageBase_V = double(parameters(4));
            currentBase_A = double(parameters(5));
            sampleTime_s = double(parameters(6));
            electricalFrequency_Hz = double(parameters(7));
            cutoffRatio = double(parameters(8));
            halfSampleTime_s = sampleTime_s / 2;
            nominalFrequency_radps = 2 * pi * 6 * electricalFrequency_Hz;
            prewarpedFrequency_radps = ...
                tan(nominalFrequency_radps * halfSampleTime_s) / ...
                halfSampleTime_s;
            cutoffFrequency_radps = cutoffRatio * prewarpedFrequency_radps;
            resistanceOverInductance = resistance_Ohm / inductance_H;

            continuousA = [ ...
                -resistanceOverInductance - 2 * cutoffFrequency_radps, ...
                -2 * cutoffFrequency_radps, ...
                2 * cutoffFrequency_radps * resistanceOverInductance / ...
                    prewarpedFrequency_radps; ...
                0, 0, prewarpedFrequency_radps; ...
                -prewarpedFrequency_radps, -prewarpedFrequency_radps, 0];
            continuousB = [ ...
                voltageBase_V / (inductance_H * currentBase_A), ...
                2 * cutoffFrequency_radps; ...
                0, 0; ...
                0, prewarpedFrequency_radps];
            identityMatrix = eye(3);
            discreteA = (identityMatrix - halfSampleTime_s * continuousA) \ ...
                (identityMatrix + halfSampleTime_s * continuousA);
            discreteB = (identityMatrix - halfSampleTime_s * continuousA) \ ...
                (2 * halfSampleTime_s * continuousB);
            augmentedMatrix = [discreteA, discreteB];

            continuousC = [ ...
                -2 * cutoffFrequency_radps * inductance_H * ...
                    currentBase_A / voltageBase_V, ...
                -2 * cutoffFrequency_radps * inductance_H * ...
                    currentBase_A / voltageBase_V, ...
                2 * cutoffFrequency_radps * resistance_Ohm * currentBase_A / ...
                    (prewarpedFrequency_radps * voltageBase_V)];
            currentFeedthrough = 2 * cutoffFrequency_radps * inductance_H * ...
                currentBase_A / voltageBase_V;
            outputRow = [continuousC, 0, currentFeedthrough];
        end

        function [allFinite, statuses] = runFrequencySweep()
            sampleCount = 401;
            frequencies = single(linspace(800 * 4 / 60, ...
                4000 * 4 / 60, sampleCount));
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            control = single([0; 1; 0.1]);
            state = zeros(8, 1, 'single');
            previousApplied = zeros(2, 1, 'single');
            statuses = zeros(sampleCount, 1, 'single');
            allFinite = true;
            for sampleIndex = 1:sampleCount
                parameters(7) = frequencies(sampleIndex);
                phase = single(2 * pi * (sampleIndex - 1) / 47);
                current = single([0.08 * sin(phase); 0.12 * cos(phase)]);
                rawPi = single([0.18 * sin(phase + single(0.2)); ...
                    0.22 * cos(phase - single(0.1))]);
                [command, estimate, state, statuses(sampleIndex)] = ...
                    rrcDobDiscreteReferenceStep(state, previousApplied, ...
                    current, rawPi, parameters, control);
                previousApplied = min(max(command, single(-0.9)), single(0.9));
                allFinite = allFinite && all(isfinite(command)) && ...
                    all(isfinite(estimate)) && all(isfinite(state));
            end
        end

        function [allFinite, statuses] = runApsfsmLikeReplay()
            sampleCount = 120;
            parameters = rrcDobDiscreteStepTest.defaultParameters();
            control = single([0; 1; 0.1]);
            state = zeros(8, 1, 'single');
            previousApplied = zeros(2, 1, 'single');
            currentQ = single(0);
            statuses = zeros(sampleCount, 1, 'single');
            allFinite = true;
            for sampleIndex = 1:sampleCount
                speedTick = floor((sampleIndex - 1) / 10);
                iqReference = single(0.04 * mod(speedTick, 4));
                currentQ = currentQ + single(0.15) * ...
                    (iqReference - currentQ);
                current = single([0.01 * sin(single(sampleIndex) * ...
                    single(0.1)); currentQ]);
                rawPi = single([0.02; 0.6 * (iqReference - currentQ)]);
                [command, estimate, state, statuses(sampleIndex)] = ...
                    rrcDobDiscreteReferenceStep(state, previousApplied, ...
                    current, rawPi, parameters, control);
                previousApplied = min(max(command, single(-0.9)), single(0.9));
                allFinite = allFinite && all(isfinite(command)) && ...
                    all(isfinite(estimate)) && all(isfinite(state));
            end
        end
    end
end
