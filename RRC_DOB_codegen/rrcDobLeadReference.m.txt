function [output, nextPrevious, nextValid, coefficients] = rrcDobLeadReference(input, previous, valid, omega, lead_us, Ts_us, reset)
%RRCDOBLEADREFERENCE Independent double-precision sixth-harmonic FIR reference.
% input/previous: raw d/q estimates, same voltage unit; omega: rad/sample.
% State belongs to caller. Reset/first valid sample passes through, then stores
% the current RAW estimate. No ramp, sign, saturation or plant feedback here.
alpha = double(lead_us) / double(Ts_us);
if lead_us == 0
    coefficients = [1, 0];
elseif omega == 0
    coefficients = [1 + alpha, alpha];
else
    coefficients = [sin((1 + alpha)*omega), sin(alpha*omega)] / sin(omega);
end
if reset || ~valid || lead_us == 0
    output = double(input);
else
    output = coefficients(1)*double(input) - coefficients(2)*double(previous);
end
nextPrevious = double(input);
nextValid = true;
end
