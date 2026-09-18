function batchResult = powerfit_run_batch(inputSpec, cfg)
%POWERFIT_RUN_BATCH Discover MF4 files, append their data, fit, and plot.
%
% Examples:
%   result = powerfit_run_batch("D:\\A_PRJ\\infineon3in1\\ape");
%   result = powerfit_run_batch(["testA.MF4" "testB.MF4"], cfg);
%
% The function never edits m4_rte.c or an A2L file.  It returns a release gate
% so that adding a candidate to firmware remains a separate reviewed action.

if nargin < 2 || isempty(cfg)
    cfg = powerfit_default_config();
end
if nargin < 1 || isempty(inputSpec)
    inputSpec = cfg.InputFolder;
end
if strlength(string(cfg.FitGroupId)) == 0
    cfg.FitGroupId = cfg.Profile.FitGroupId;
elseif string(cfg.Profile.FitGroupId) == "UNSPECIFIED"
    cfg.Profile.FitGroupId = cfg.FitGroupId;
elseif string(cfg.Profile.FitGroupId) ~= string(cfg.FitGroupId)
    error("powerfit:FitGroup", ...
        "cfg.FitGroupId and cfg.Profile.FitGroupId must identify the same campaign.");
end

mf4Files = localDiscoverMdfFiles(inputSpec, cfg.FilePatterns);
[powerfitDatabase, importReport] = powerfit_update_database(mf4Files, cfg);

batchResult = struct();
batchResult.Status = "Imported";
batchResult.Config = cfg;
batchResult.DatabaseFile = string(cfg.DatabaseFile);
batchResult.ImportReport = importReport;
batchResult.DatabaseManifest = powerfitDatabase.Manifest;
batchResult.Fit = struct("Status", "NotRun", "Reason", "");
batchResult.Figures = gobjects(0, 1);
batchResult.PerSourceFigures = gobjects(0, 1);

try
    fitResult = powerfit_fit_loss_model(powerfitDatabase, cfg);
    batchResult.Fit = fitResult;
    if cfg.SaveFitResult
        resultFolder = fileparts(cfg.FitResultFile);
        if ~isfolder(resultFolder)
            mkdir(resultFolder);
        end
        save(char(cfg.FitResultFile), "fitResult", "importReport", "cfg", "-v7.3");
    end
    if cfg.CreateFigures
        batchResult.Figures = powerfit_plot_report(fitResult);
    end
    if cfg.CreatePerSourceFigures
        batchResult.PerSourceFigures = powerfit_plot_each_source(fitResult);
    end
    if cfg.PrintProgress
        fprintf("Fit group %s: %d samples, Pdc R^2 %.4f, RMSE %.1f W.\n", ...
            fitResult.FitGroupId, fitResult.Metrics.Samples, ...
            fitResult.Metrics.PdcRSquared, fitResult.Metrics.RMSE_W);
        if fitResult.CanApplyToFirmware
            fprintf("Release gate passed. A separate reviewed firmware update is still required.\n");
        else
            fprintf("Release gate blocked: %s\n", ...
                strjoin(cellstr(fitResult.ReleaseBlockReasons), "; "));
        end
    end
catch ME
    batchResult.Status = "ImportedWithoutFit";
    batchResult.Fit = struct("Status", "NotRun", "Reason", string(ME.message));
    warning("powerfit:FitNotRun", "Samples were imported, but fit was not run: %s", ME.message);
end
end

function mf4Files = localDiscoverMdfFiles(inputSpec, filePatterns)
inputSpec = string(inputSpec);
if isscalar(inputSpec) && isfolder(inputSpec)
    filePatterns = string(filePatterns);
    records = dir(fullfile(inputSpec, "__powerfit_no_matching_file__"));
    for patternIndex = 1:numel(filePatterns)
        records = [records; dir(fullfile(inputSpec, filePatterns(patternIndex)))]; %#ok<AGROW>
    end
    if isempty(records)
        error("powerfit:NoMdfFiles", ...
            "No MF4 files matching [%s] were found in %s.", ...
            strjoin(cellstr(filePatterns), ", "), inputSpec);
    end
    files = string(fullfile({records.folder}, {records.name}));
    [~, uniqueIndex] = unique(lower(files), "stable");
    mf4Files = sort(files(uniqueIndex)).';
    return
end

inputSpec = inputSpec(:);
if isempty(inputSpec) || ~all(isfile(inputSpec))
    error("powerfit:InputSpec", "Pass one MF4 folder or one or more existing MF4 file paths.");
end
mf4Files = inputSpec;
end
