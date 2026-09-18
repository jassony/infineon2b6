classdef apsfsmTorqueCompensationStepTest < matlab.unittest.TestCase
    %apsfsmTorqueCompensationStepTest Deterministic APSFSM core/policy tests.

    methods (Test)
        function t01OutputPrecedesStateUpdate(testCase)
            parameters = testCase.coreParameters();
            covariance = single(0.5) * parameters(2) * parameters(2);
            state = single([0.02; -0.01; 0.3; covariance]);
            speedReference = single(0.201);
            speedFeedback = single(0.2);
            control = single([1; 0; 0]);

            expectedRaw = state(1) * sin(state(3)) + ...
                state(2) * cos(state(3));
            expectedTheta = mod(state(3) + single(5.0e-4) * ...
                parameters(1) * speedFeedback, single(2.0 * pi));
            expectedCovariance = parameters(4) * covariance + ...
                single(0.5) * parameters(2) * parameters(2);
            expectedB = state(1) + parameters(2) * ...
                sin(expectedTheta + parameters(3)) * ...
                (speedReference - speedFeedback) / expectedCovariance;
            expectedC = state(2) + parameters(2) * ...
                cos(expectedTheta + parameters(3)) * ...
                (speedReference - speedFeedback) / expectedCovariance;

            [stateNext, diagnostics] = apsfsmTorqueCompensationStep( ...
                state, speedReference, speedFeedback, parameters, control);

            testCase.verifyEqual(diagnostics(1), expectedRaw, ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(stateNext(1), expectedB, ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(stateNext(2), expectedC, ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(stateNext(3), expectedTheta, ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(stateNext(4), expectedCovariance, ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(diagnostics(2:3), single([1; 2]));
        end

        function t02OffResetAndIneligibleClearState(testCase)
            parameters = testCase.coreParameters();
            initialState = testCase.initialCoreState(parameters);
            learnedState = single([0.04; -0.03; 1.2; 0.2]);

            [offState, offDiagnostics] = apsfsmTorqueCompensationStep( ...
                learnedState, single(0.2), single(0.19), parameters, ...
                single([0; 0; 0]));
            [resetState, resetDiagnostics] = apsfsmTorqueCompensationStep( ...
                learnedState, single(0.2), single(0.19), parameters, ...
                single([1; 0; 1]));

            calibration = testCase.policyCalibration(single(0));
            inputs = testCase.policyInputs();
            policyState = testCase.initialPolicyState(calibration);
            policyState(1:3) = learnedState(1:3);
            [policyStateNext, policyDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                policyState, inputs, calibration);

            testCase.verifyEqual(offState, initialState);
            testCase.verifyEqual(resetState, initialState);
            testCase.verifyEqual(offDiagnostics(1:3), single([0; 0; 0]));
            testCase.verifyEqual(resetDiagnostics(1:3), single([0; 0; 0]));
            testCase.verifyEqual(policyStateNext, ...
                testCase.initialPolicyState(calibration));
            testCase.verifyEqual(policyDiagnostics(2), inputs(4));
            testCase.verifyEqual(policyDiagnostics(13), single(0));
        end

        function t03SpeedWindowBoundariesAreInclusive(testCase)
            calibration = testCase.policyCalibration(single(1));
            calibration(10) = single(1);
            initialState = testCase.initialPolicyState(calibration);
            inputs = testCase.policyInputs();

            inputs(1:2) = calibration(8);
            [~, lowDiagnostics] = apsfsmTorqueCompensationReference( ...
                initialState, inputs, calibration);
            inputs(1:2) = calibration(9);
            [~, highDiagnostics] = apsfsmTorqueCompensationReference( ...
                initialState, inputs, calibration);
            inputs(1:2) = single(799 / 10000);
            [lowResetState, belowDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                initialState, inputs, calibration);
            inputs(1:2) = single(4001 / 10000);
            [highResetState, aboveDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                initialState, inputs, calibration);

            testCase.verifyEqual(lowDiagnostics(10), single(1));
            testCase.verifyEqual(highDiagnostics(10), single(1));
            testCase.verifyEqual(lowDiagnostics(13), single(2));
            testCase.verifyEqual(highDiagnostics(13), single(2));
            testCase.verifyEqual(belowDiagnostics(13), single(1));
            testCase.verifyEqual(aboveDiagnostics(13), single(1));
            testCase.verifyEqual(lowResetState, initialState);
            testCase.verifyEqual(highResetState, initialState);
        end

        function t04ReverseAndZeroSpeedNeverLearn(testCase)
            calibration = testCase.policyCalibration(single(2));
            initialState = testCase.initialPolicyState(calibration);
            inputs = testCase.policyInputs();

            inputs(1:2) = single(0);
            [zeroState, zeroDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                initialState, inputs, calibration);
            inputs(1:2) = single(-0.2);
            [reverseState, reverseDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                initialState, inputs, calibration);

            testCase.verifyEqual(zeroState, initialState);
            testCase.verifyEqual(reverseState, initialState);
            testCase.verifyEqual(zeroDiagnostics(10:12), ...
                single([0; 0; 0]));
            testCase.verifyEqual(reverseDiagnostics(10:12), ...
                single([0; 0; 0]));
            testCase.verifyEqual(zeroDiagnostics(13), single(1));
            testCase.verifyEqual(reverseDiagnostics(13), single(1));
        end

        function t05InvalidCalibrationAndNumericsFailClosed(testCase)
            parameters = testCase.coreParameters();
            initialCoreState = testCase.initialCoreState(parameters);
            invalidParameters = parameters;
            invalidParameters(4) = single(NaN);
            [invalidParameterState, invalidParameterDiagnostics] = ...
                apsfsmTorqueCompensationStep( ...
                initialCoreState, single(0.2), single(0.2), ...
                invalidParameters, single([1; 0; 0]));

            [invalidInputState, invalidInputDiagnostics] = ...
                apsfsmTorqueCompensationStep( ...
                initialCoreState, single(NaN), single(0.2), ...
                parameters, single([1; 0; 0]));
            invalidState = initialCoreState;
            invalidState(1) = single(Inf);
            [invalidNumericState, invalidNumericDiagnostics] = ...
                apsfsmTorqueCompensationStep( ...
                invalidState, single(0.2), single(0.2), parameters, ...
                single([1; 0; 0]));
            invalidPolicyCalibration = ...
                testCase.policyCalibration(single(1));
            invalidPolicyCalibration(5) = single(0.999);
            [invalidConvergenceState, invalidConvergenceDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                testCase.initialPolicyState(invalidPolicyCalibration), ...
                testCase.policyInputs(), invalidPolicyCalibration);
            invalidPolicyCalibration(5) = single(0.98);
            invalidPolicyCalibration(6:7) = single(0);
            [invalidLimitState, invalidLimitDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                testCase.initialPolicyState(invalidPolicyCalibration), ...
                testCase.policyInputs(), invalidPolicyCalibration);

            testCase.verifyEqual(invalidParameterState, ...
                zeros(4, 1, 'single'));
            testCase.verifyEqual(invalidParameterDiagnostics(1:3), ...
                single([0; 0; 4]));
            testCase.verifyEqual(invalidInputState, initialCoreState);
            testCase.verifyEqual(invalidInputDiagnostics(1:3), ...
                single([0; 0; 5]));
            testCase.verifyEqual(invalidNumericState, initialCoreState);
            testCase.verifyEqual(invalidNumericDiagnostics(1:3), ...
                single([0; 0; 5]));
            testCase.verifyEqual(invalidConvergenceState, ...
                zeros(7, 1, 'single'));
            testCase.verifyEqual(invalidConvergenceDiagnostics(13), ...
                single(4));
            testCase.verifyEqual(invalidLimitState, zeros(7, 1, 'single'));
            testCase.verifyEqual(invalidLimitDiagnostics(13), single(4));
        end

        function t06MechanicalAngleWrapsForLongRun(testCase)
            parameters = testCase.coreParameters();
            state = testCase.initialCoreState(parameters);
            control = single([1; 0; 0]);

            for sampleIndex = 1:50000
                [state, diagnostics] = apsfsmTorqueCompensationStep( ...
                    state, single(0.2), single(0.2), parameters, control);
            end

            testCase.verifyGreaterThanOrEqual(state(3), single(0));
            testCase.verifyLessThan(state(3), single(2.0 * pi));
            testCase.verifyTrue(all(isfinite(state)));
            testCase.verifyEqual(diagnostics(2:3), single([1; 2]));
        end

        function t07CoefficientVectorIsProjected(testCase)
            parameters = testCase.coreParameters();
            parameters(5) = single(0.05);
            state = testCase.initialCoreState(parameters);

            [stateNext, diagnostics] = apsfsmTorqueCompensationStep( ...
                state, single(1.0), single(0.1), parameters, ...
                single([1; 0; 0]));
            magnitude = hypot(stateNext(1), stateNext(2));

            testCase.verifyLessThanOrEqual(magnitude, parameters(5));
            testCase.verifyEqual(magnitude, parameters(5), ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(diagnostics(2:3), single([1; 2]));
        end

        function t08ExternalClippingFreezesLearningButAdvancesAngle(testCase)
            calibration = testCase.policyCalibration(single(2));
            calibration(10:11) = single(1);
            state = testCase.initialPolicyState(calibration);
            state(1) = single(0.05);
            state(3) = single(pi / 2);
            inputs = testCase.policyInputs();
            inputs(4) = single(0.79);
            inputs(7:9) = single([-0.8; 0.8; 1.0]);

            [stateNext, diagnostics] = ...
                apsfsmTorqueCompensationReference(state, inputs, calibration);

            expectedTheta = mod(state(3) + single(5.0e-4) * ...
                calibration(2) * inputs(2), single(2.0 * pi));
            testCase.verifyEqual(diagnostics(2), single(0.8), ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(diagnostics(14), single(1));
            testCase.verifyEqual(stateNext([1, 2, 4]), state([1, 2, 4]));
            testCase.verifyEqual(stateNext(3), expectedTheta, ...
                'AbsTol', single(2.0e-7));
        end

        function t09ShadowToApplyRetainsLearningAndRamps(testCase)
            shadowCalibration = testCase.policyCalibration(single(1));
            shadowCalibration(10) = single(1);
            shadowCalibration(11) = single(1000);
            state = testCase.initialPolicyState(shadowCalibration);
            state(1:2) = single([0.02; -0.01]);
            inputs = testCase.policyInputs();
            inputs(4) = single(0.1);
            inputs(1:2) = single(0.2);

            [shadowState, shadowDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                state, inputs, shadowCalibration);
            applyCalibration = shadowCalibration;
            applyCalibration(1) = single(2);
            [applyState, firstApplyDiagnostics] = ...
                apsfsmTorqueCompensationReference( ...
                shadowState, inputs, applyCalibration);
            for sampleIndex = 2:1000
                [applyState, finalApplyDiagnostics] = ...
                    apsfsmTorqueCompensationReference( ...
                    applyState, inputs, applyCalibration);
            end

            testCase.verifyEqual(shadowDiagnostics(2), inputs(4));
            testCase.verifyEqual(shadowDiagnostics(4), single(0));
            testCase.verifyEqual(shadowDiagnostics(13), single(2));
            testCase.verifyEqual(firstApplyDiagnostics(15), single(0.001), ...
                'AbsTol', single(2.0e-7));
            testCase.verifyEqual(finalApplyDiagnostics(15), single(1), ...
                'AbsTol', single(2.0e-6));
            testCase.verifyEqual(applyState(1:2), shadowState(1:2), ...
                'AbsTol', single(2.0e-6));
            testCase.verifyEqual(finalApplyDiagnostics(13), single(3));
        end

        function t10SourceReplayMatchesRecordedTerminalValues(testCase)
            testDirectory = fileparts(mfilename('fullpath'));
            replay = load(fullfile(testDirectory, 'testdata', ...
                'apsfsm_source_replay_vectors.mat'));
            source = replay.source;
            parameters = single([source.omegaBase_radps; source.kHat; ...
                source.rho_rad; source.lambda; source.coefficientLimit_PU]);
            state = testCase.initialCoreState(parameters);
            iqRaw = zeros(size(replay.speedFeedback_PU), 'single');

            for sampleIndex = 1:numel(iqRaw)
                publishedState = state;
                learnEnable = single(replay.learnEnable_u8(sampleIndex));
                control = single([learnEnable; 0; 1 - learnEnable]);
                [state, diagnostics] = apsfsmTorqueCompensationStep( ...
                    state, replay.speedReference_PU(sampleIndex), ...
                    replay.speedFeedback_PU(sampleIndex), parameters, ...
                    control);
                iqRaw(sampleIndex) = diagnostics(1);
            end
            terminalHarmonicPeak = hypot(publishedState(1), ...
                publishedState(2));

            testCase.verifyEqual(publishedState(1), ...
                single(source.goldenBHat_PU), ...
                'AbsTol', single(1.0e-4));
            testCase.verifyEqual(publishedState(2), ...
                single(source.goldenCHat_PU), ...
                'AbsTol', single(1.0e-4));
            testCase.verifyEqual(terminalHarmonicPeak, ...
                single(source.goldenIqPeak_PU), 'AbsTol', single(1.0e-4));
            testCase.verifyTrue(all(isfinite(iqRaw)));
        end

        function zeroCovarianceRecoversAtWrapperStartup(testCase)
            parameters = testCase.coreParameters();
            state = zeros(4, 1, 'single');

            [stateNext, diagnostics] = apsfsmTorqueCompensationStep( ...
                state, single(0.2), single(0.2), parameters, ...
                single([1; 0; 0]));

            minimumCovariance = single(0.5) * parameters(2) * parameters(2);
            expectedCovariance = parameters(4) * minimumCovariance + ...
                minimumCovariance;
            testCase.verifyEqual(stateNext(4), expectedCovariance, ...
                'AbsTol', single(2.0e-7));
            testCase.verifyTrue(all(isfinite(stateNext)));
            testCase.verifyEqual(diagnostics(2:3), single([1; 2]));
        end
    end

    methods (Static, Access = private)
        function parameters = coreParameters()
            omegaBase = single(2.0 * pi * 10000.0 / 60.0);
            parameters = single([omegaBase; 0.25; -pi / 2; 0.98; 0.05]);
        end

        function state = initialCoreState(parameters)
            state = zeros(4, 1, 'single');
            state(4) = single(0.5) * parameters(2) * parameters(2);
        end

        function calibration = policyCalibration(mode)
            omegaBase = single(2.0 * pi * 10000.0 / 60.0);
            calibration = single([mode; omegaBase; 0.25; -pi / 2; 0.98; ...
                -0.05; 0.05; 0.08; 0.40; 1000; 1000]);
        end

        function state = initialPolicyState(calibration)
            state = zeros(7, 1, 'single');
            state(4) = single(0.5) * calibration(3) * calibration(3);
        end

        function inputs = policyInputs()
            inputs = single([0.2; 0.2; 0.0; 0.0; 1; 0; -0.8; 0.8; 0.8]);
        end
    end
end
