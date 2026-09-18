function result = powerfit_run_closedloop_rated_350v_20260907()
%POWERFIT_RUN_CLOSEDLOOP_RATED_350V_20260907 Refit the supplied rated logs.
%
% The fit uses only rows where the FOC state machine is run/closed-loop
% (Meas_FocState == 4 and Meas_FocSubState == 2).  It keeps the earlier
% steady-state database and result intact by using new local MAT-file names.

cfg = powerfit_default_config();
toolFolder = fileparts(mfilename("fullpath"));
cfg.InputFolder = "D:\A_PRJ\infineon3in1\power-fit\data";
cfg.FilePatterns = "*.MF4";
cfg.Profile.FitGroupId = "rated_350V_20260907_closedloop";
cfg.FitGroupId = cfg.Profile.FitGroupId;
cfg.DatabaseFile = fullfile(toolFolder, "local_data", ...
    "rated_350V_20260907_closedloop_database.mat");
cfg.FitResultFile = fullfile(toolFolder, "local_data", ...
    "rated_350V_20260907_closedloop_fit_result.mat");
cfg.FitFilter.Enable = true;
cfg.FitFilter.Method = "movmean-zero-phase";
cfg.FitFilter.Window_s = 1.00;
cfg.CreateFigures = true;
cfg.CreatePerSourceFigures = true;
cfg.PrintProgress = true;

result = powerfit_closedloop_run_batch(cfg.InputFolder, cfg);
end
