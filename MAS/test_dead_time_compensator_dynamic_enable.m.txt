% Reference checks for dynamic dead-time compensation enable hysteresis.

q15 = 32768;
assert(round(0.06 / 50.0 * q15) == 39);
factorQ15 = round(2 * 750 * 20000 / 1e9 * q15);
assert(factorQ15 == 983);
assert(isequal(deadTimeCompensatorQ15([1000 -1000 0], 16384, factorQ15), [491 -491 491]));

dynamicEnable = false;
[dynamicEnable, effectiveEnable] = deadTimeEnable(false, 3500, dynamicEnable);
assert(dynamicEnable && effectiveEnable);

[dynamicEnable, effectiveEnable] = deadTimeEnable(false, 3800, dynamicEnable);
assert(dynamicEnable && effectiveEnable);

[dynamicEnable, effectiveEnable] = deadTimeEnable(false, 4000, dynamicEnable);
assert(~dynamicEnable && ~effectiveEnable);

[dynamicEnable, effectiveEnable] = deadTimeEnable(false, 3800, dynamicEnable);
assert(~dynamicEnable && ~effectiveEnable);

[dynamicEnable, effectiveEnable] = deadTimeEnable(false, -3500, dynamicEnable);
assert(dynamicEnable && effectiveEnable);

[dynamicEnable, effectiveEnable] = deadTimeEnable(false, -4000, dynamicEnable);
assert(~dynamicEnable && ~effectiveEnable);

[dynamicEnable, effectiveEnable] = deadTimeEnable(true, 4500, dynamicEnable);
assert(~dynamicEnable && effectiveEnable);

fprintf('Dynamic dead-time enable checks passed.\n');

function [dynamicEnable, effectiveEnable] = deadTimeEnable(forceEnable, speedCommandRpm, dynamicEnable)
absoluteSpeedRpm = abs(speedCommandRpm);
if absoluteSpeedRpm <= 3500
    dynamicEnable = true;
elseif absoluteSpeedRpm >= 4000
    dynamicEnable = false;
end
effectiveEnable = forceEnable || dynamicEnable;
end

function compensationQ15 = deadTimeCompensatorQ15(currentsQ15, dcLinkVoltageQ15, factorQ15)
signCurrents = ones(1, 3);
signCurrents(currentsQ15 < 0) = -1;
compensationQ15 = floor(double(dcLinkVoltageQ15) * double(factorQ15) / 32768) .* signCurrents;
end
