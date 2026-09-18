function batchResult = powerfit_closedloop_refit_batch(inputSpec, cfg)
%POWERFIT_CLOSEDLOOP_REFIT_BATCH Create the DB if needed, then fit closed loop.
%
% Fit selection remains exactly State == 4 and SubState == 2.

if nargin < 2 || isempty(cfg)
    cfg = powerfit_default_config();
end
if nargin < 1 || isempty(inputSpec)
    inputSpec = cfg.InputFolder;
end
cfg.SchemaVersion = "1.1-closedloop";
cfg.Channel.FocSubState = "Meas_FocSubState";
cfg.FocRunState = 4;
cfg.FocClosedLoopSubState = 2;
if isfield(cfg, "EnableSteadyStateGate")
    cfg.EnableSteadyStateGate = false;
end
powerfit_closedloop_initialize_database(cfg);
batchResult = powerfit_closedloop_run_batch(inputSpec, cfg);
end
