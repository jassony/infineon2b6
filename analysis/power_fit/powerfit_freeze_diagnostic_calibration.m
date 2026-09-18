function frozen = powerfit_freeze_diagnostic_calibration(fitResultFile, snapshotFile)
%POWERFIT_FREEZE_DIAGNOSTIC_CALIBRATION Freeze a replay-only power-fit set.
%
% The snapshot is intentionally diagnostic-only.  It preserves the exact
% fitted coefficients, state-machine selection rule, filter configuration,
% source manifest, and release-gate result without writing any XCP or
% firmware calibration variable.

arguments
    fitResultFile (1,1) string
    snapshotFile (1,1) string
end

if ~isfile(fitResultFile)
    error("powerfit:FrozenCalibrationInput", ...
        "Fit-result file does not exist: %s", fitResultFile);
end
if isfile(snapshotFile)
    error("powerfit:FrozenCalibrationExists", ...
        "Refusing to overwrite frozen snapshot: %s", snapshotFile);
end

loaded = load(fitResultFile, "cfg", "fitResult", "importReport");
requiredFields = ["cfg", "fitResult", "importReport"];
if ~all(isfield(loaded, requiredFields))
    error("powerfit:FrozenCalibrationInput", ...
        "Fit-result file lacks one or more required variables.");
end

coefficientNames = ["K_CU"; "K_FE"; "K_INV_COND"; "K_INV_SW"];
coefficients = loaded.fitResult.Coefficients;
if ~isequal(string(coefficients.Coefficient), coefficientNames)
    error("powerfit:FrozenCalibrationCoefficients", ...
        "Unexpected coefficient order in the fit result.");
end

frozen = struct();
frozen.SchemaVersion = "1.0";
frozen.Status = "DIAGNOSTIC_ONLY_NOT_FIRMWARE_APPROVED";
frozen.FrozenAt = datetime("now", "TimeZone", "Asia/Shanghai");
frozen.SourceFitResultFile = fitResultFile;
frozen.Selection = struct( ...
    "Rule", "Meas_FocState == 4 & Meas_FocSubState == 2", ...
    "FocRunState", double(loaded.cfg.FocRunState), ...
    "FocClosedLoopSubState", double(loaded.cfg.FocClosedLoopSubState));
frozen.ModelDefinition = loaded.fitResult.ModelDefinition;
frozen.Coefficients = coefficients;
frozen.CoefficientVector = double(coefficients.Value(:));
frozen.Filter = loaded.fitResult.Filter;
frozen.Metrics = loaded.fitResult.Metrics;
frozen.SourceCoverage = loaded.fitResult.SourceCoverage;
frozen.ReleaseGate = loaded.fitResult.ReleaseGate;
frozen.CanApplyToFirmware = logical(loaded.fitResult.CanApplyToFirmware);
frozen.ReleaseBlockReasons = string(loaded.fitResult.ReleaseBlockReasons);
frozen.ImportReport = loaded.importReport;
frozen.Config = loaded.cfg;

snapshotFolder = string(fileparts(snapshotFile));
if strlength(snapshotFolder) > 0 && ~isfolder(snapshotFolder)
    mkdir(snapshotFolder);
end
save(snapshotFile, "frozen", "-v7.3");

fprintf("Frozen diagnostic calibration: %s\n", snapshotFile);
fprintf("Firmware application permitted: %d\n", frozen.CanApplyToFirmware);
end
