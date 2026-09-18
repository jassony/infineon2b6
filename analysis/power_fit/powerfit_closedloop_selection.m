function closedLoop = powerfit_closedloop_selection(focState, focSubState, cfg)
%POWERFIT_CLOSEDLOOP_SELECTION Return the state-machine closed-loop rows only.
%
% This contains the sole fit-selection rule: run state plus closed-loop
% substate.  No dwell, derivative, power-direction, or steady-state test is
% applied here.

closedLoop = double(focState(:)) == double(cfg.FocRunState) ...
    & double(focSubState(:)) == double(cfg.FocClosedLoopSubState);
end
