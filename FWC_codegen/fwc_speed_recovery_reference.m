function [state, targetQ15, integralQ24] = fwc_speed_recovery_reference(state, active, referenceQ15, estimatedQ15, previousIqQ15, requestQ15, errorQ14, iqRequestQ15, iqFeedbackQ15, integralBeforeQ24, integralAfterQ24, kawTs, kawFormat, iqLoQ15, iqHiQ15)
%FWC_SPEED_RECOVERY_REFERENCE Independent 500 us recovery integration reference.
% State: [active; speed magnitude cap Q15; filtered Iq in Q15 counts], single.
% The normal PI and acceleration limiter are external, unchanged components.
% targetQ15 is sent through the normal ramp, never assigned as its output.
% integralQ24 is the NEXT-cycle PI state; the current Iq output is untouched.
state = single(state);
if ~active
    state(:) = single(0);
elseif state(1) == 0
    state(1) = single(1);
    state(2) = min(single(32767), min(abs(single(referenceQ15)), abs(single(estimatedQ15))));
    state(3) = single(previousIqQ15);
end
targetQ15 = single(requestQ15);
integralQ24 = single(integralAfterQ24);
if state(1) == 0
    return
end
targetQ15 = min(state(2), max(-state(2), targetQ15));
alpha = single(0.5) / single(20.5);
state(3) = state(3) + alpha * (single(iqFeedbackQ15) - state(3));
mismatch = single(iqRequestQ15) - state(3);
outward = (errorQ14 > 0 && mismatch > 0) || (errorQ14 < 0 && mismatch < 0);
if ~outward
    return
end
if (errorQ14 > 0 && integralAfterQ24 > integralBeforeQ24) || ...
        (errorQ14 < 0 && integralAfterQ24 < integralBeforeQ24)
    integralQ24 = single(integralBeforeQ24);
end
gain = single(0);
if kawTs > 0 && kawFormat <= 30
    gain = min(single(1), single(kawTs) / single(2^double(kawFormat)));
end
integralQ24 = integralQ24 - gain * mismatch * single(512);
integralQ24 = min(single(iqHiQ15)*single(512), max(single(iqLoQ15)*single(512), integralQ24));
integralQ24 = fix(integralQ24);
end
