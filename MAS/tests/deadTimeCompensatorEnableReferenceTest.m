classdef deadTimeCompensatorEnableReferenceTest < matlab.unittest.TestCase
    methods (Test)
        function testDefaultParameterMapping(testCase)
            currentBandQ15 = round(0.06 / 50.0 * 32768);
            factorQ15 = round(2 * 750 * 20000 / 1e9 * 32768);

            testCase.verifyEqual(currentBandQ15, 39);
            testCase.verifyEqual(factorQ15, 983);
        end

        function testLowSpeedEnablesDefaultSpeedGate(testCase)
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(true, false, true, 3500, false);

            testCase.verifyTrue(dynamicEnable);
            testCase.verifyTrue(effectiveEnable);
        end

        function testHighSpeedDisablesDefaultSpeedGate(testCase)
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(true, false, true, 4000, true);

            testCase.verifyFalse(dynamicEnable);
            testCase.verifyFalse(effectiveEnable);
        end

        function testHysteresisHoldsEnabledWithinBand(testCase)
            [initialDynamicEnable, ~] = deadTimeCompensatorEnableReference(true, false, true, 3500, false);
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(true, false, true, 3800, initialDynamicEnable);

            testCase.verifyTrue(dynamicEnable);
            testCase.verifyTrue(effectiveEnable);
        end

        function testHysteresisHoldsDisabledWithinBand(testCase)
            [initialDynamicEnable, ~] = deadTimeCompensatorEnableReference(true, false, true, 4000, true);
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(true, false, true, 3800, initialDynamicEnable);

            testCase.verifyFalse(dynamicEnable);
            testCase.verifyFalse(effectiveEnable);
        end

        function testReverseSpeedUsesMagnitude(testCase)
            [enabledAtLowSpeed, effectiveAtLowSpeed] = deadTimeCompensatorEnableReference(true, false, true, -3500, false);
            [enabledAtHighSpeed, effectiveAtHighSpeed] = deadTimeCompensatorEnableReference(true, false, true, -4000, enabledAtLowSpeed);

            testCase.verifyTrue(enabledAtLowSpeed);
            testCase.verifyTrue(effectiveAtLowSpeed);
            testCase.verifyFalse(enabledAtHighSpeed);
            testCase.verifyFalse(effectiveAtHighSpeed);
        end

        function testMasterDisableWins(testCase)
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(false, false, true, 3500, false);

            testCase.verifyTrue(dynamicEnable);
            testCase.verifyFalse(effectiveEnable);
        end

        function testForceOffWins(testCase)
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(true, true, true, 3500, false);

            testCase.verifyTrue(dynamicEnable);
            testCase.verifyFalse(effectiveEnable);
        end

        function testSpeedGateCanBeBypassed(testCase)
            [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference(true, false, false, 4500, false);

            testCase.verifyFalse(dynamicEnable);
            testCase.verifyTrue(effectiveEnable);
        end
    end
end

function [dynamicEnable, effectiveEnable] = deadTimeCompensatorEnableReference( ...
    totalEnable, forceOff, speedLogicEnable, speedCommandRpm, dynamicEnable)
absoluteSpeedRpm = abs(speedCommandRpm);

if absoluteSpeedRpm <= 3500
    dynamicEnable = true;
elseif absoluteSpeedRpm >= 4000
    dynamicEnable = false;
end

effectiveEnable = totalEnable && ~forceOff && (~speedLogicEnable || dynamicEnable);
end
