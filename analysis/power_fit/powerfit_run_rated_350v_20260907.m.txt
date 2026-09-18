function result = powerfit_run_rated_350v_20260907()
%POWERFIT_RUN_RATED_350V_20260907 Process the supplied 350 V rated campaign.
%
% Hardware/firmware identity and measurement confirmations are deliberately
% left unconfirmed.  Fill them in from bench evidence before considering any
% candidate for a firmware calibration review.

cfg = powerfit_default_config();
toolFolder = fileparts(mfilename("fullpath"));
cfg.InputFolder = "D:\A_PRJ\infineon3in1\power-fit\data";
cfg.FilePatterns = "*.MF4";
cfg.Profile.FitGroupId = "rated_350V_20260907";
cfg.FitGroupId = cfg.Profile.FitGroupId;
cfg.DatabaseFile = fullfile(toolFolder, "local_data", ...
    "rated_350V_20260907_database.mat");
cfg.FitResultFile = fullfile(toolFolder, "local_data", ...
    "rated_350V_20260907_fit_result.mat");

% Rated-point traces are intended for steady-state calibration.  A bounded
% 1.0 s, zero-phase fit filter suppresses measurement ripple without adding
% phase delay; raw residuals remain a separate release-gate diagnostic.
cfg.EnableSteadyStateGate = true;
cfg.FitFilter.Enable = true;
cfg.FitFilter.Method = "movmean-zero-phase";
cfg.FitFilter.Window_s = 1.00;
cfg.CreatePerSourceFigures = true;
cfg.PrintProgress = true;

result = powerfit_run_batch(cfg.InputFolder, cfg);
end
