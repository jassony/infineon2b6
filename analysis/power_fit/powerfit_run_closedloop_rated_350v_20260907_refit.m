function result = powerfit_run_closedloop_rated_350v_20260907_refit()
%POWERFIT_RUN_CLOSEDLOOP_RATED_350V_20260907_REFIT Refit the 9 supplied logs.

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

result = powerfit_closedloop_refit_batch(cfg.InputFolder, cfg);
end
