classdef CVACIFController < matlab.System
    %CVACIFCONTROLLER Discrete current-vector-angle I-f startup controller.
    %
    % The external interface uses the same per-unit convention as the
    % SensorlessIFPMSM example.  All paper equations are evaluated once in
    % SI units inside this object.  This implementation is intended for MIL
    % simulation only; it is not a deployment-ready controller.

    properties
        TsFast = 50e-6
        TsSlow = 500e-6
        PolePairs = 3
        SpeedBaseRpm = 1500
        CurrentBaseA = 7.6367532368
        VoltageBaseV = 311.76914536
        LqH = 92.3e-3
        FluxWb = 0.67
        IStartA = 3.8183766184
        IAlignA = 1.1455129855
        IMinA = 0.3818376618
        OmegaRefRadps = 125.66370614
        OmegaThetaOnRadps = 18.849555922
        BetaSeedRadps2 = 150
        BetaMaxRadps2 = 200
        DeltaOmegaMaxRadps = 12.566370614
        Ktheta = 3.989088
        Kdp = 0.05635005736
        TorqueMinNm = 1.1512405504
        HpfCutoffHz = 20
        KpBeta = 717.34477785
        KiBeta = 14130.213549
        KawBeta = 19.697938823
        KpCurrent = 6.6968413319
        KiCurrent = 1640.0139656
        KawCurrent = 244.89365722
        CurrentRiseAps = 76.367532368
        CurrentFallAps = 38.183766184
        AlignTimeS = 0.5
        VectorRotateTimeS = 0.02
        ThetaValidTimeS = 0.02
        ConstSpeedMinTimeS = 0.1
        HandoffQualifyTimeS = 0.05
        StartupTimeoutS = 5
        ThetaEstimateLimitRad = pi / 6
        ThetaFaultLimitRad = pi / 3
        ConstSpeedAngleLimitRad = 15 * pi / 180
        HandoffAngleLimitRad = 5 * pi / 180
        HandoffSpeedLimitPU = 0.02
        HandoffUnderspeedLimitPU = 0.005
        CurrentTrackingLimitA = 0.3 * 3.8183766184
        CurrentTrackingTimeS = 0.1
        VoltageSaturationPU = 0.98
        VoltageSaturationTimeS = 0.1
    end

    properties (DiscreteState)
        State
        FastCounter
        StateTicks
        StartupTicks
        ThetaValidTicks
        ThetaFaultTicks
        CurrentFaultTicks
        VoltageFaultTicks
        QualifyTicks
        ThetaRad
        OmegaI0Radps
        PowerLowPassW
        BetaIntegrator
        CurrentIntegrator
        IqReferenceA
        ThetaErrorRad
        LastIq0PU
        PreviousSlowSpeedPU
        AbortCode
    end

    methods (Access = protected)
        function setupImpl(obj)
            resetImpl(obj);
        end

        function resetImpl(obj)
            obj.State = uint8(0);
            obj.FastCounter = uint16(0);
            obj.StateTicks = uint32(0);
            obj.StartupTicks = uint32(0);
            obj.ThetaValidTicks = uint32(0);
            obj.ThetaFaultTicks = uint32(0);
            obj.CurrentFaultTicks = uint32(0);
            obj.VoltageFaultTicks = uint32(0);
            obj.QualifyTicks = uint32(0);
            obj.ThetaRad = single(0);
            obj.OmegaI0Radps = single(0);
            obj.PowerLowPassW = single(0);
            obj.BetaIntegrator = single(0);
            obj.CurrentIntegrator = single(0);
            obj.IqReferenceA = single(0);
            obj.ThetaErrorRad = single(0);
            obj.LastIq0PU = single(0);
            obj.PreviousSlowSpeedPU = single(0);
            obj.AbortCode = uint8(0);
        end

        function [posOutPU, iq0PU, enableSpeedLoop, idqOutPU, diagnostics] = ...
                stepImpl(obj, enable, posObsPU, speedRefPU, speedObsPU, ...
                         iqPU, idqClosedPU, vqPU, vdPU)
            enabled = logical(enable ~= 0);
            posObsPU = single(posObsPU);
            speedRefPU = single(speedRefPU);
            speedObsPU = single(speedObsPU);
            iqPU = single(iqPU);
            vqPU = single(vqPU);
            vdPU = single(vdPU);

            vdV = vdPU * single(obj.VoltageBaseV);
            vqV = vqPU * single(obj.VoltageBaseV);
            iqA = iqPU * single(obj.CurrentBaseA);

            if ~enabled
                resetImpl(obj);
                [posOutPU, iq0PU, enableSpeedLoop, idqOutPU] = ...
                    disabledOutputs();
                diagnostics = makeDiagnostics(obj, single(0), false, ...
                    single(0), single(0), single(0), single(0), false);
                return;
            end

            if obj.State == uint8(0)
                obj.State = uint8(1); % Align
                obj.StateTicks = uint32(0);
                obj.StartupTicks = uint32(0);
                obj.ThetaRad = single(0);
                obj.IqReferenceA = single(obj.IStartA);
            end

            parametersValid = obj.TsFast > 0 && obj.TsSlow >= obj.TsFast && ...
                obj.PolePairs > 0 && obj.SpeedBaseRpm > 0 && ...
                obj.CurrentBaseA > 0 && obj.VoltageBaseV > 0 && ...
                obj.LqH > 0 && obj.FluxWb > 0 && obj.IStartA > 0;
            inputsFinite = all(isfinite([posObsPU, speedRefPU, speedObsPU, ...
                iqPU, vqPU, vdPU, single(idqClosedPU(1)), ...
                single(idqClosedPU(2))]));

            if ~parametersValid || ~inputsFinite
                setAbort(obj, uint8(1));
            elseif speedRefPU < 0
                setAbort(obj, uint8(2));
            end

            powerW = single(1.5) * vqV * iqA;
            omegaH = single(2 * pi * obj.HpfCutoffHz);
            dPowerWps = omegaH * (powerW - obj.PowerLowPassW);
            alpha = single(exp(-2 * pi * obj.HpfCutoffHz * obj.TsFast));
            obj.PowerLowPassW = alpha * obj.PowerLowPassW + ...
                (single(1) - alpha) * powerW;

            torqueEstimateNm = max(single(1.5 * obj.PolePairs * obj.FluxWb) * ...
                abs(obj.IqReferenceA), single(obj.TorqueMinNm));
            deltaOmegaRadps = saturateSingle(-single(obj.Kdp) * dPowerWps / ...
                torqueEstimateNm, -single(obj.DeltaOmegaMaxRadps), ...
                single(obj.DeltaOmegaMaxRadps));
            if obj.State < uint8(4)
                deltaOmegaRadps = single(0);
            end
            omegaIRadps = max(single(0), obj.OmegaI0Radps + deltaOmegaRadps);

            if obj.State >= uint8(5)
                thetaCurrentThreshold = single(0.8 * obj.IMinA);
            else
                thetaCurrentThreshold = single(0.8 * obj.IStartA);
            end
            thetaValid = omegaIRadps >= single(obj.OmegaThetaOnRadps) && ...
                abs(iqA) >= thetaCurrentThreshold && ...
                isfinite(omegaIRadps) && isfinite(vdV) && isfinite(iqA);
            thetaRawRad = single(0);
            if thetaValid
                denominator = omegaIRadps * single(obj.FluxWb);
                thetaRawRad = (-omegaIRadps * single(obj.LqH) * iqA - vdV) / ...
                    denominator;
                thetaValid = isfinite(thetaRawRad);
            end

            if thetaValid
                obj.ThetaErrorRad = saturateSingle(thetaRawRad, ...
                    -single(obj.ThetaEstimateLimitRad), ...
                    single(obj.ThetaEstimateLimitRad));
                obj.ThetaValidTicks = obj.ThetaValidTicks + uint32(1);
            else
                obj.ThetaValidTicks = uint32(0);
            end

            monitorActive = obj.State >= uint8(4) && obj.State <= uint8(6);
            if monitorActive && thetaValid && ...
                    abs(thetaRawRad) > single(obj.ThetaFaultLimitRad)
                obj.ThetaFaultTicks = obj.ThetaFaultTicks + uint32(1);
            else
                obj.ThetaFaultTicks = uint32(0);
            end

            currentMonitorActive = obj.State >= uint8(3) && obj.State <= uint8(6);
            if currentMonitorActive && ...
                    abs(iqA - obj.IqReferenceA) > ...
                    single(obj.CurrentTrackingLimitA)
                obj.CurrentFaultTicks = obj.CurrentFaultTicks + uint32(1);
            else
                obj.CurrentFaultTicks = uint32(0);
            end

            voltageMagnitudePU = hypot(vdV, vqV) / ...
                single(obj.VoltageBaseV);
            if currentMonitorActive && ...
                    voltageMagnitudePU > single(obj.VoltageSaturationPU)
                obj.VoltageFaultTicks = obj.VoltageFaultTicks + uint32(1);
            else
                obj.VoltageFaultTicks = uint32(0);
            end

            if obj.ThetaFaultTicks >= timeToFastTicks(obj, 0.05)
                setAbort(obj, uint8(3));
            elseif obj.CurrentFaultTicks >= ...
                    timeToFastTicks(obj, obj.CurrentTrackingTimeS)
                setAbort(obj, uint8(4));
            elseif obj.VoltageFaultTicks >= ...
                    timeToFastTicks(obj, obj.VoltageSaturationTimeS)
                setAbort(obj, uint8(5));
            end

            if obj.State >= uint8(1) && obj.State <= uint8(6)
                obj.StartupTicks = obj.StartupTicks + uint32(1);
                if obj.StartupTicks >= timeToFastTicks(obj, obj.StartupTimeoutS)
                    setAbort(obj, uint8(6));
                end
            end

            obj.FastCounter = obj.FastCounter + uint16(1);
            slowDivider = uint16(max(1, round(obj.TsSlow / obj.TsFast)));
            if obj.FastCounter >= slowDivider
                obj.FastCounter = uint16(0);
                runSlowController(obj, thetaValid, posObsPU, speedObsPU);
            end

            if obj.State >= uint8(3) && obj.State <= uint8(6)
                obj.ThetaRad = wrapTwoPi(obj.ThetaRad + ...
                    single(obj.TsFast) * omegaIRadps);
            end

            [posOutPU, iq0PU, enableSpeedLoop, idqOutPU] = ...
                stateOutputs(obj, posObsPU, idqClosedPU);
            handoffReady = obj.State == uint8(6) && obj.QualifyTicks > 0;
            diagnostics = makeDiagnostics(obj, thetaRawRad, thetaValid, ...
                omegaIRadps, deltaOmegaRadps, powerW, dPowerWps, handoffReady);

        end

        function num = getNumInputsImpl(~)
            num = 8;
        end

        function num = getNumOutputsImpl(~)
            num = 5;
        end

        function direct = isInputDirectFeedthroughImpl(~, ~)
            % The integrating full model provides explicit Unit Delay blocks
            % on Iq/Vq/Vd before this controller boundary.
            direct = true;
        end

        function [s1, s2, s3, s4, s5] = getOutputSizeImpl(~)
            s1 = [1 1];
            s2 = [1 1];
            s3 = [1 1];
            s4 = [2 1];
            s5 = [16 1];
        end

        function [t1, t2, t3, t4, t5] = getOutputDataTypeImpl(~)
            t1 = 'single';
            t2 = 'single';
            t3 = 'logical';
            t4 = 'single';
            t5 = 'single';
        end

        function [c1, c2, c3, c4, c5] = isOutputComplexImpl(~)
            c1 = false;
            c2 = false;
            c3 = false;
            c4 = false;
            c5 = false;
        end

        function [f1, f2, f3, f4, f5] = isOutputFixedSizeImpl(~)
            f1 = true;
            f2 = true;
            f3 = true;
            f4 = true;
            f5 = true;
        end

        function [stateSize, dataType, complexity] = ...
                getDiscreteStateSpecificationImpl(~, propertyName)
            stateSize = [1 1];
            complexity = false;
            switch propertyName
                case {'State', 'AbortCode'}
                    dataType = 'uint8';
                case 'FastCounter'
                    dataType = 'uint16';
                case {'StateTicks', 'StartupTicks', 'ThetaValidTicks', ...
                      'ThetaFaultTicks', 'CurrentFaultTicks', ...
                      'VoltageFaultTicks', 'QualifyTicks'}
                    dataType = 'uint32';
                otherwise
                    dataType = 'single';
            end
        end

        function [n1, n2, n3, n4, n5, n6, n7, n8] = getInputNamesImpl(~)
            n1 = 'Enable';
            n2 = 'PosObsPU';
            n3 = 'SpeedRefPU';
            n4 = 'SpeedObsPU';
            n5 = 'IqPU';
            n6 = 'IdqClosedPU';
            n7 = 'VqPU';
            n8 = 'VdPU';
        end

        function [n1, n2, n3, n4, n5] = getOutputNamesImpl(~)
            n1 = 'PosOutPU';
            n2 = 'Iq0PU';
            n3 = 'EnableSpeedLoop';
            n4 = 'IdqOutPU';
            n5 = 'Diagnostics';
        end
    end

    methods (Access = private)
        function runSlowController(obj, thetaValid, posObsPU, speedObsPU)
            obj.StateTicks = obj.StateTicks + uint32(1);
            speedNondecreasing = single(speedObsPU) >= ...
                obj.PreviousSlowSpeedPU - single(1e-6);
            obj.PreviousSlowSpeedPU = single(speedObsPU);
            switch obj.State
                case uint8(1) % Align
                    if obj.StateTicks >= timeToSlowTicks(obj, obj.AlignTimeS)
                        enterState(obj, uint8(2));
                    end

                case uint8(2) % VectorRotate
                    if obj.StateTicks >= timeToSlowTicks(obj, obj.VectorRotateTimeS)
                        obj.OmegaI0Radps = single(0);
                        obj.IqReferenceA = single(obj.IStartA);
                        enterState(obj, uint8(3));
                    end

                case uint8(3) % BlindLaunch
                    obj.OmegaI0Radps = min(single(obj.OmegaRefRadps), ...
                        obj.OmegaI0Radps + single(obj.TsSlow * obj.BetaSeedRadps2));
                    if obj.ThetaValidTicks >= ...
                            timeToFastTicks(obj, obj.ThetaValidTimeS)
                        errorBeta = obj.ThetaErrorRad;
                        obj.BetaIntegrator = single(obj.BetaSeedRadps2) - ...
                            single(obj.KpBeta) * errorBeta;
                        enterState(obj, uint8(4));
                    end

                case uint8(4) % AccelCVAC
                    errorBeta = obj.ThetaErrorRad;
                    betaUnsat = single(obj.KpBeta) * errorBeta + ...
                        obj.BetaIntegrator;
                    beta = saturateSingle(betaUnsat, single(0), ...
                        single(obj.BetaMaxRadps2));
                    obj.BetaIntegrator = obj.BetaIntegrator + single(obj.TsSlow) * ...
                        (single(obj.KiBeta) * errorBeta + ...
                         single(obj.KawBeta) * (beta - betaUnsat));
                    obj.OmegaI0Radps = min(single(obj.OmegaRefRadps), ...
                        obj.OmegaI0Radps + single(obj.TsSlow) * beta);
                    targetSpeedPU = single(obj.OmegaRefRadps) / ...
                        single(obj.PolePairs * 2*pi/60 * obj.SpeedBaseRpm);
                    if obj.OmegaI0Radps >= single(obj.OmegaRefRadps) && ...
                            thetaValid && ...
                            single(speedObsPU) >= single(0.9) * targetSpeedPU
                        obj.OmegaI0Radps = single(obj.OmegaRefRadps);
                        errorCurrent = -obj.ThetaErrorRad;
                        obj.CurrentIntegrator = -single(obj.KpCurrent) * errorCurrent;
                        obj.IqReferenceA = single(obj.IStartA);
                        enterState(obj, uint8(5));
                    end

                case uint8(5) % ConstSpeedCVAC
                    updateCurrentController(obj);
                    if obj.StateTicks >= ...
                            timeToSlowTicks(obj, obj.ConstSpeedMinTimeS) && ...
                            thetaValid && ...
                            abs(obj.ThetaErrorRad) < ...
                            single(obj.ConstSpeedAngleLimitRad)
                        enterState(obj, uint8(6));
                    end

                case uint8(6) % HandoffQualify
                    updateCurrentController(obj);
                    angleError = wrapPi((single(posObsPU) - ...
                        obj.ThetaRad / single(2 * pi)) * single(2 * pi));
                    speedIfPU = obj.OmegaI0Radps / single(obj.PolePairs * ...
                        2 * pi / 60 * obj.SpeedBaseRpm);
                    lightLoad = obj.IqReferenceA <= single(obj.IMinA + ...
                        0.05 * obj.IStartA);
                    underspeedReady = ~lightLoad || ...
                        single(speedObsPU) - speedIfPU >= ...
                        -single(obj.HandoffUnderspeedLimitPU);
                    ready = thetaValid && ...
                        abs(obj.ThetaErrorRad) < single(obj.HandoffAngleLimitRad) && ...
                        abs(angleError) < single(obj.HandoffAngleLimitRad) && ...
                        abs(single(speedObsPU) - speedIfPU) < ...
                        single(obj.HandoffSpeedLimitPU) && ...
                        underspeedReady && speedNondecreasing;
                    if ready
                        obj.QualifyTicks = obj.QualifyTicks + uint32(1);
                        if obj.QualifyTicks >= ...
                                timeToSlowTicks(obj, obj.HandoffQualifyTimeS)
                            obj.LastIq0PU = obj.IqReferenceA / ...
                                single(obj.CurrentBaseA);
                            enterState(obj, uint8(7));
                        end
                    else
                        obj.QualifyTicks = uint32(0);
                    end

                otherwise
                    % ClosedLoop and Abort remain latched until Enable is false.
            end
        end

        function updateCurrentController(obj)
            errorCurrent = -obj.ThetaErrorRad;
            deltaUnsat = single(obj.KpCurrent) * errorCurrent + ...
                obj.CurrentIntegrator;
            currentUnsat = single(obj.IStartA) + deltaUnsat;
            currentCommand = saturateSingle(currentUnsat, single(obj.IMinA), ...
                single(obj.IStartA));
            obj.CurrentIntegrator = obj.CurrentIntegrator + single(obj.TsSlow) * ...
                (single(obj.KiCurrent) * errorCurrent + ...
                 single(obj.KawCurrent) * (currentCommand - currentUnsat));

            if currentCommand > obj.IqReferenceA
                maxStep = single(obj.CurrentRiseAps * obj.TsSlow);
                obj.IqReferenceA = min(currentCommand, obj.IqReferenceA + maxStep);
            else
                maxStep = single(obj.CurrentFallAps * obj.TsSlow);
                obj.IqReferenceA = max(currentCommand, obj.IqReferenceA - maxStep);
            end
        end

        function [posOutPU, iq0PU, enableSpeedLoop, idqOutPU] = ...
                stateOutputs(obj, posObsPU, idqClosedPU)
            enableSpeedLoop = false;
            posOutPU = wrapUnit(obj.ThetaRad / single(2 * pi));

            switch obj.State
                case uint8(1) % Align
                    idqOutPU = single([obj.IAlignA; 0]) / ...
                        single(obj.CurrentBaseA);

                case uint8(2) % VectorRotate
                    totalTicks = max(uint32(1), ...
                        timeToSlowTicks(obj, obj.VectorRotateTimeS));
                    progress = min(single(1), single(obj.StateTicks) / ...
                        single(totalTicks));
                    magnitude = single(obj.IAlignA) + progress * ...
                        single(obj.IStartA - obj.IAlignA);
                    angle = progress * single(pi / 2);
                    idqOutPU = single([magnitude * cos(angle); ...
                        magnitude * sin(angle)]) / single(obj.CurrentBaseA);

                case {uint8(3), uint8(4)} % BlindLaunch / AccelCVAC
                    idqOutPU = single([0; obj.IStartA]) / ...
                        single(obj.CurrentBaseA);

                case {uint8(5), uint8(6)} % Constant speed / qualify
                    idqOutPU = single([0; obj.IqReferenceA]) / ...
                        single(obj.CurrentBaseA);

                case uint8(7) % ClosedLoop
                    enableSpeedLoop = true;
                    posOutPU = wrapUnit(single(posObsPU));
                    idqOutPU = single([idqClosedPU(1); idqClosedPU(2)]);

                otherwise % Disabled / Abort
                    idqOutPU = single([0; 0]);
            end

            if obj.State == uint8(7)
                iq0PU = obj.LastIq0PU;
            else
                iq0PU = idqOutPU(2);
                obj.LastIq0PU = iq0PU;
            end
        end

        function diagnostics = makeDiagnostics(obj, thetaRaw, thetaValid, ...
                omegaI, deltaOmega, powerW, dPower, handoffReady)
            diagnostics = single(zeros(16, 1));
            diagnostics(1) = single(obj.State);
            diagnostics(2) = obj.ThetaErrorRad;
            diagnostics(3) = single(thetaRaw);
            diagnostics(4) = single(thetaValid);
            diagnostics(5) = obj.OmegaI0Radps;
            diagnostics(6) = single(omegaI);
            diagnostics(7) = single(deltaOmega);
            diagnostics(8) = obj.IqReferenceA;
            diagnostics(9) = single(powerW);
            diagnostics(10) = single(dPower);
            diagnostics(11) = single(handoffReady);
            diagnostics(12) = single(obj.AbortCode);
            diagnostics(13) = single(obj.ConstSpeedAngleLimitRad);
            diagnostics(14) = single(obj.StateTicks);
            diagnostics(15) = single(thetaValid && ...
                abs(obj.ThetaErrorRad) < single(obj.ConstSpeedAngleLimitRad));
            diagnostics(16) = single(timeToSlowTicks(obj, ...
                obj.ConstSpeedMinTimeS));
        end

        function setAbort(obj, code)
            obj.State = uint8(8);
            obj.AbortCode = uint8(code);
            obj.QualifyTicks = uint32(0);
        end

        function enterState(obj, state)
            obj.State = uint8(state);
            obj.StateTicks = uint32(0);
            if state ~= uint8(6)
                obj.QualifyTicks = uint32(0);
            end
        end

        function ticks = timeToFastTicks(obj, timeS)
            ticks = uint32(max(1, ceil(double(timeS) / double(obj.TsFast))));
        end

        function ticks = timeToSlowTicks(obj, timeS)
            ticks = uint32(max(1, ceil(double(timeS) / double(obj.TsSlow))));
        end
    end
end

function [posOutPU, iq0PU, enableSpeedLoop, idqOutPU] = disabledOutputs()
posOutPU = single(0);
iq0PU = single(0);
enableSpeedLoop = false;
idqOutPU = single([0; 0]);
end

function value = saturateSingle(value, lower, upper)
value = min(max(single(value), single(lower)), single(upper));
end

function angle = wrapTwoPi(angle)
angle = single(mod(double(angle), 2 * pi));
end

function angle = wrapPi(angle)
angle = single(mod(double(angle) + pi, 2 * pi) - pi);
end

function value = wrapUnit(value)
value = single(mod(double(value), 1));
end

