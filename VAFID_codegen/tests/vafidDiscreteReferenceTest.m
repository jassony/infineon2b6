classdef vafidDiscreteReferenceTest < matlab.unittest.TestCase
    %VAFIDDISCRETEREFERENCETEST Deterministic VAFID reference tests.

    methods (TestClassSetup)
        function addReferenceFolderToPath(testCase)
            import matlab.unittest.fixtures.PathFixture
            referenceFolder = fileparts(fileparts(mfilename('fullpath')));
            testCase.applyFixture(PathFixture(referenceFolder));
        end
    end

    methods (TestMethodSetup)
        function clearPersistentState(~)
            clear vafid_discrete_reference_step
        end
    end

    methods (Test)
        function testDefaultModeIsInert(testCase)
            config = vafid_default_config();
            output = vafidDiscreteReferenceTest.callStep( ...
                uint8(0), false, true, zeros(1, 7, 'single'), config);

            testCase.verifyEqual(output.probeD_PU, single(0), AbsTol=single(0));
            testCase.verifyEqual(output.probeQ_PU, single(0), AbsTol=single(0));
            testCase.verifyFalse(output.valid);
            testCase.verifyTrue(output.stale);
            testCase.verifyEqual(output.status, uint8(0));
            testCase.verifyEqual(output.windowSamples, uint32(0));
        end

        function testResetClearsAcquisitionAndEstimate(testCase)
            config = vafid_default_config();
            config.settleTime_s = single(0);
            config.windowLength_samples = uint32(16);
            vafidDiscreteReferenceTest.runConstantSamples(config, 7);

            output = vafidDiscreteReferenceTest.callStep( ...
                uint8(1), true, true, zeros(1, 7, 'single'), config);

            testCase.verifyEqual(output.status, uint8(1));
            testCase.verifyEqual(output.windowSamples, uint32(0));
            testCase.verifyEqual(output.acceptedWindows, uint16(0));
            testCase.verifyFalse(output.valid);
            testCase.verifyTrue(output.stale);
            testCase.verifyEqual(output.rs_Ohm, config.nominalRs_Ohm, ...
                AbsTol=single(0));
        end

        function testSettleAndWindowTimingAreExact(testCase)
            config = vafid_default_config();
            config.settleTime_s = single(5) * config.sampleTime_s;
            config.windowLength_samples = uint32(8);

            trace = vafidDiscreteReferenceTest.runConstantSamples(config, 13);

            testCase.verifyEqual(trace.status(1:5), repmat(uint8(2), 5, 1));
            testCase.verifyEqual(trace.status(6:12), repmat(uint8(3), 7, 1));
            testCase.verifyTrue(ismember(trace.status(13), uint8([5, 10])));
            testCase.verifyEqual(trace.windowSamples(12), uint32(7));
            testCase.verifyEqual(trace.windowSamples(13), uint32(0));
        end

        function testInvalidSampleRestartsContinuousWindow(testCase)
            config = vafid_default_config();
            config.settleTime_s = single(2) * config.sampleTime_s;
            config.windowLength_samples = uint32(8);
            vafidDiscreteReferenceTest.runConstantSamples(config, 6);
            invalidSignals = zeros(1, 7, 'single');
            invalidSignals(1) = single(NaN);

            invalidOutput = vafidDiscreteReferenceTest.callStep( ...
                uint8(1), false, true, invalidSignals, config);
            restartedTrace = vafidDiscreteReferenceTest.runConstantSamples(config, 3);

            testCase.verifyEqual(invalidOutput.status, uint8(8));
            testCase.verifyEqual(invalidOutput.windowSamples, uint32(0));
            testCase.verifyTrue(invalidOutput.stale);
            testCase.verifyEqual(restartedTrace.status(1:2), repmat(uint8(2), 2, 1));
            testCase.verifyEqual(restartedTrace.status(3), uint8(3));
            testCase.verifyEqual(restartedTrace.windowSamples(3), uint32(1));
        end

        function testNoiselessSyntheticPlantRecoversParameters(testCase)
            config = vafid_default_config();
            truth = struct('rs', single(0.62), 'ld', single(1.10e-3), ...
                'lq', single(1.65e-3), 'flux', single(52.0e-3), ...
                'omega', single(420.0), 'idMean', single(-2.0));

            output = vafidDiscreteReferenceTest.runSyntheticPlant( ...
                config, truth, 22);

            testCase.verifyTrue(output.valid);
            testCase.verifyFalse(output.stale);
            testCase.verifyLessThan(output.condition, config.conditionLimit);
            testCase.verifyLessThan(output.residual, single(2.0e-3));
            testCase.verifyEqual(output.rs_Ohm, truth.rs, RelTol=single(0.02));
            testCase.verifyEqual(output.ld_H, truth.ld, RelTol=single(0.02));
            testCase.verifyEqual(output.lq_H, truth.lq, RelTol=single(0.02));
            testCase.verifyEqual(output.fluxPM_Wb, truth.flux, AbsTol=single(5.0e-4));
        end

        function testIllConditionedWindowIsRejected(testCase)
            config = vafid_default_config();
            config.settleTime_s = single(0);
            config.windowLength_samples = uint32(20);

            trace = vafidDiscreteReferenceTest.runConstantSamples(config, 20);

            testCase.verifyTrue(ismember(trace.status(end), uint8([5, 10])));
            testCase.verifyFalse(trace.valid(end));
            testCase.verifyTrue(trace.stale(end));
            testCase.verifyEqual(trace.acceptedWindows(end), uint16(0));
        end

        function testNaNInputIsRejectedImmediately(testCase)
            config = vafid_default_config();
            signals = zeros(1, 7, 'single');
            signals(7) = single(NaN);

            output = vafidDiscreteReferenceTest.callStep( ...
                uint8(1), false, true, signals, config);

            testCase.verifyEqual(output.status, uint8(8));
            testCase.verifyEqual(output.probeD_PU, single(0), AbsTol=single(0));
            testCase.verifyEqual(output.probeQ_PU, single(0), AbsTol=single(0));
            testCase.verifyFalse(output.valid);
            testCase.verifyTrue(output.stale);
        end

        function testFluxEstimateRemovesSaliencyContribution(testCase)
            config = vafid_default_config();
            config.parameterFusion = single(1.0);
            config.fluxFusion = single(1.0);
            truth = struct('rs', single(0.70), 'ld', single(2.20e-3), ...
                'lq', single(0.80e-3), 'flux', single(48.0e-3), ...
                'omega', single(360.0), 'idMean', single(-8.0));
            activeFluxMean = truth.flux + (truth.ld - truth.lq) * truth.idMean;

            output = vafidDiscreteReferenceTest.runSyntheticPlant( ...
                config, truth, 5);

            testCase.verifyTrue(output.valid);
            testCase.verifyEqual(output.fluxPM_Wb, truth.flux, AbsTol=single(6.0e-4));
            testCase.verifyGreaterThan(abs(output.fluxPM_Wb - activeFluxMean), ...
                single(8.0e-3));
        end
    end

    methods (Static, Access = private)
        function output = callStep(mode, resetRequest, sampleValid, signals, config)
            [output.probeD_PU, output.probeQ_PU, output.rs_Ohm, ...
                output.ld_H, output.lq_H, output.fluxPM_Wb, output.valid, ...
                output.fresh, output.stale, output.status, output.condition, ...
                output.residual, output.acceptedWindows, output.windowSamples] = ...
                vafid_discrete_reference_step(mode, resetRequest, sampleValid, ...
                signals(1), signals(2), signals(3), signals(4), signals(5), ...
                signals(6), signals(7), config);
        end

        function trace = runConstantSamples(config, numberOfSamples)
            trace.status = zeros(numberOfSamples, 1, 'uint8');
            trace.windowSamples = zeros(numberOfSamples, 1, 'uint32');
            trace.valid = false(numberOfSamples, 1);
            trace.stale = true(numberOfSamples, 1);
            trace.acceptedWindows = zeros(numberOfSamples, 1, 'uint16');
            signals = zeros(1, 7, 'single');
            for sampleIndex = 1:numberOfSamples
                output = vafidDiscreteReferenceTest.callStep( ...
                    uint8(1), false, true, signals, config);
                trace.status(sampleIndex) = output.status;
                trace.windowSamples(sampleIndex) = output.windowSamples;
                trace.valid(sampleIndex) = output.valid;
                trace.stale(sampleIndex) = output.stale;
                trace.acceptedWindows(sampleIndex) = output.acceptedWindows;
            end
        end

        function output = runSyntheticPlant(config, truth, numberOfWindows)
            settleSamples = round(double(config.settleTime_s / config.sampleTime_s));
            totalSamples = settleSamples + numberOfWindows * ...
                double(config.windowLength_samples);
            time_s = single((0:(totalSamples - 1)).') * config.sampleTime_s;
            omegaD = single(2 * pi) * config.probeD_Frequency_Hz;
            omegaQ = single(2 * pi) * config.probeQ_Frequency_Hz;
            id_A = truth.idMean + single(0.60) * cos(omegaD * time_s - single(0.20)) + ...
                single(0.25) * cos(omegaQ * time_s + single(0.40));
            iq_A = single(3.0) + single(0.30) * cos(omegaD * time_s + single(0.50)) + ...
                single(0.70) * cos(omegaQ * time_s - single(0.35));
            did_Aps = -single(0.60) * omegaD * sin(omegaD * time_s - single(0.20)) - ...
                single(0.25) * omegaQ * sin(omegaQ * time_s + single(0.40));
            diq_Aps = -single(0.30) * omegaD * sin(omegaD * time_s + single(0.50)) - ...
                single(0.70) * omegaQ * sin(omegaQ * time_s - single(0.35));
            vd_V = truth.rs * id_A + truth.ld * did_Aps - truth.omega * truth.lq * iq_A;
            vq_V = truth.rs * iq_A + truth.lq * diq_Aps + ...
                truth.omega * (truth.ld * id_A + truth.flux);
            angle_rad = mod(truth.omega * time_s + single(pi), ...
                single(2 * pi)) - single(pi);
            cosAngle = cos(angle_rad);
            sinAngle = sin(angle_rad);
            voltageAlpha_V = cosAngle .* vd_V - sinAngle .* vq_V;
            voltageBeta_V = sinAngle .* vd_V + cosAngle .* vq_V;
            currentAlpha_A = cosAngle .* id_A - sinAngle .* iq_A;
            currentBeta_A = sinAngle .* id_A + cosAngle .* iq_A;
            activeFlux_Wb = truth.flux + (truth.ld - truth.lq) .* id_A;

            output = struct();
            for sampleIndex = 1:totalSamples
                signals = [voltageAlpha_V(sampleIndex), voltageBeta_V(sampleIndex), ...
                    currentAlpha_A(sampleIndex), currentBeta_A(sampleIndex), ...
                    angle_rad(sampleIndex), truth.omega, activeFlux_Wb(sampleIndex)];
                output = vafidDiscreteReferenceTest.callStep( ...
                    uint8(1), false, true, signals, config);
            end
        end
    end
end
