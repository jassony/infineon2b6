function test_dead_time_compensator_reference
% Offline reference for the standard Dead-Time Compensator equation.

q15 = 32768;
deadTime_ns = 1500;
switchingFrequency_Hz = 20000;
factorQ15 = round(2 * deadTime_ns * switchingFrequency_Hz / 1e9 * q15);

assert(factorQ15 == 1966);
assert(isequal(deadTimeCompensatorQ15([1000 -1000 0], 16384, factorQ15), [983 -983 983]));
assert(isequal(deadTimeCompensatorQ15([1000 -1000 0], 0, factorQ15), [0 0 0]));
assert(isequal(deadTimeCompensatorQ15([1000 -1000 0], 16384, 0), [0 0 0]));

fprintf('Dead-time compensator reference checks passed.\n');
end

function vabcCompQ15 = deadTimeCompensatorQ15(iabcQ15, vdcQ15, factorQ15)
signI = ones(1, 3);
signI(iabcQ15 < 0) = -1;
vabcCompQ15 = floor(double(vdcQ15) * double(factorQ15) / 32768) .* signI;
end
