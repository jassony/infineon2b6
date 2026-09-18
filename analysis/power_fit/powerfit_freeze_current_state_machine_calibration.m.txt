function frozen = powerfit_freeze_current_state_machine_calibration()
%POWERFIT_FREEZE_CURRENT_STATE_MACHINE_CALIBRATION Freeze the current fit.
%
% Create a non-overwritable diagnostic snapshot from the closed-loop-only
% rated-350 V result.  Run this before collecting a comparison MF4 file.

analysisFolder = string(fileparts(mfilename("fullpath")));
localDataFolder = fullfile(analysisFolder, "local_data");
fitResultFile = fullfile(localDataFolder, ...
    "rated_350V_20260907_state_machine_closedloop_fit_result.mat");
snapshotFile = fullfile(localDataFolder, ...
    "rated_350V_20260907_state_machine_closedloop_frozen_diagnostic.mat");

frozen = powerfit_freeze_diagnostic_calibration(fitResultFile, snapshotFile);
end
