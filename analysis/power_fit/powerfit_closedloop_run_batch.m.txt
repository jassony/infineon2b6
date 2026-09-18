function batchResult = powerfit_closedloop_run_batch(inputSpec, cfg)
%POWERFIT_CLOSEDLOOP_RUN_BATCH Import and fit MF4 data from closed-loop rows.
%
% The sole sample-selection expression is:
%   Meas_FocState == 4 & Meas_FocSubState == 2
% Filtering remains an offline signal treatment after selection; it never
% changes FitEligible.  The closed-loop database is separate from earlier
% steady-state databases so their traceability is preserved.

if nargin < 2 || isempty(cfg)
    cfg = powerfit_default_config();
    toolFolder = fileparts(mfilename("fullpath"));
    cfg.DatabaseFile = fullfile(toolFolder, "local_data", ...
        "powerfit_closedloop_database.mat");
    cfg.FitResultFile = fullfile(toolFolder, "local_data", ...
        "powerfit_closedloop_fit_result.mat");
end
if nargin < 1 || isempty(inputSpec)
    inputSpec = cfg.InputFolder;
end

cfg = localConfigureClosedLoop(cfg);
if strlength(string(cfg.FitGroupId)) == 0
    cfg.FitGroupId = cfg.Profile.FitGroupId;
elseif string(cfg.Profile.FitGroupId) == "UNSPECIFIED"
    cfg.Profile.FitGroupId = cfg.FitGroupId;
elseif string(cfg.Profile.FitGroupId) ~= string(cfg.FitGroupId)
    error("powerfit:FitGroup", ...
        "cfg.FitGroupId and cfg.Profile.FitGroupId must identify the same campaign.");
end

mf4Files = localDiscoverMdfFiles(inputSpec, cfg.FilePatterns);
[powerfitDatabase, importReport] = localUpdateClosedLoopDatabase(mf4Files, cfg);
closedLoopRows = powerfitDatabase.Samples.FitEligible ...
    & powerfitDatabase.Samples.FitGroupId == string(cfg.FitGroupId);

batchResult = struct();
batchResult.Status = "Imported";
batchResult.Config = cfg;
batchResult.DatabaseFile = string(cfg.DatabaseFile);
batchResult.ImportReport = importReport;
batchResult.DatabaseManifest = powerfitDatabase.Manifest;
batchResult.ClosedLoopStateRows = sum(closedLoopRows);
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
        fprintf("Closed-loop rows: %d; numerical fit rows: %d; raw Pdc RMSE: %.1f W.\n", ...
            batchResult.ClosedLoopStateRows, fitResult.Metrics.Samples, ...
            fitResult.Metrics.RawPdcRMSE_W);
    end
catch ME
    batchResult.Status = "ImportedWithoutFit";
    batchResult.Fit = struct("Status", "NotRun", "Reason", string(ME.message));
    warning("powerfit:FitNotRun", ...
        "Closed-loop data were imported, but fit was not run: %s", ME.message);
end
end

function cfg = localConfigureClosedLoop(cfg)
cfg.SchemaVersion = "1.1-closedloop";
cfg.Channel.FocSubState = "Meas_FocSubState";
cfg.FocRunState = 4;
cfg.FocClosedLoopSubState = 2;
% This legacy option is consumed only by the raw importer.  The result is
% overwritten by powerfit_closedloop_attach_substate immediately afterward.
if isfield(cfg, "EnableSteadyStateGate")
    cfg.EnableSteadyStateGate = false;
end
end

function [powerfitDatabase, importReport] = localUpdateClosedLoopDatabase(mf4Files, cfg)
if isfile(cfg.DatabaseFile)
    loaded = load(cfg.DatabaseFile, "powerfitDatabase");
    if ~isfield(loaded, "powerfitDatabase")
        error("powerfit:DatabaseSchema", "Database file does not contain powerfitDatabase.");
    end
    powerfitDatabase = loaded.powerfitDatabase;
    localValidateClosedLoopDatabase(powerfitDatabase, cfg.SchemaVersion);
else
    powerfitDatabase = localEmptyClosedLoopDatabase(cfg.SchemaVersion);
end

importReport = localEmptyImportReport();
databaseChanged = false;
for fileIndex = 1:numel(mf4Files)
    source = powerfit_source_record(mf4Files(fileIndex));
    duplicate = any(powerfitDatabase.Manifest.SourceId == source.SourceId ...
        & powerfitDatabase.Manifest.FitGroupId == string(cfg.Profile.FitGroupId));
    if duplicate
        importReport = [importReport; localImportReportRow(source, "SkippedDuplicate", ...
            "SHA-256 fingerprint already exists in this closed-loop fit group.", 0)]; %#ok<AGROW>
        continue
    end

    [newSamples, manifestRow] = powerfit_extract_mf4(source.SourceFile, cfg);
    subStateGroup = NaN;
    if startsWith(manifestRow.Status, "Imported")
        try
            [newSamples, subStateGroup] = powerfit_closedloop_attach_substate( ...
                newSamples, source.SourceFile, cfg);
            manifestRow.EligibleRows = sum(newSamples.FitEligible);
        catch ME
            newSamples = powerfitDatabase.Samples([], :);
            manifestRow.Status = "Rejected";
            manifestRow.Reason = string(ME.message);
        end
    end
    manifestRow = localAddSubStateGroup(manifestRow, subStateGroup);
    powerfitDatabase.Manifest = [powerfitDatabase.Manifest; manifestRow];
    if startsWith(manifestRow.Status, "Imported")
        powerfitDatabase.Samples = [powerfitDatabase.Samples; newSamples];
    end
    closedLoopCount = 0;
    if startsWith(manifestRow.Status, "Imported")
        closedLoopCount = sum(newSamples.FitEligible);
    end
    importReport = [importReport; localImportReportRow(source, manifestRow.Status, ...
        manifestRow.Reason, closedLoopCount)]; %#ok<AGROW>
    databaseChanged = true;
end

if databaseChanged
    databaseFolder = fileparts(cfg.DatabaseFile);
    if ~isfolder(databaseFolder)
        mkdir(databaseFolder);
    end
    powerfitDatabase.UpdatedAt = datetime("now");
    save(char(cfg.DatabaseFile), "powerfitDatabase", "-v7.3");
end
end

function powerfitDatabase = localEmptyClosedLoopDatabase(schemaVersion)
powerfitDatabase = powerfit_empty_database();
powerfitDatabase.SchemaVersion = string(schemaVersion);
powerfitDatabase.Samples = addvars(powerfitDatabase.Samples, nan(0, 1), ...
    "After", "FocState", "NewVariableNames", "FocSubState");
powerfitDatabase.Manifest = addvars(powerfitDatabase.Manifest, nan(0, 1), ...
    "After", "FocStateGroup", "NewVariableNames", "FocSubStateGroup");
end

function localValidateClosedLoopDatabase(powerfitDatabase, schemaVersion)
requiredFields = ["SchemaVersion", "Samples", "Manifest"];
if ~isstruct(powerfitDatabase) || ~all(isfield(powerfitDatabase, requiredFields))
    error("powerfit:DatabaseSchema", "Closed-loop database schema is incomplete.");
end
if string(powerfitDatabase.SchemaVersion) ~= string(schemaVersion)
    error("powerfit:DatabaseSchema", ...
        "Database version %s does not match %s.", ...
        string(powerfitDatabase.SchemaVersion), string(schemaVersion));
end
if ~ismember("FocSubState", string(powerfitDatabase.Samples.Properties.VariableNames)) ...
        || ~ismember("FocSubStateGroup", ...
        string(powerfitDatabase.Manifest.Properties.VariableNames))
    error("powerfit:DatabaseSchema", ...
        "Database is missing state-machine substate fields; use a new closed-loop database file.");
end
end

function manifestRow = localAddSubStateGroup(manifestRow, subStateGroup)
manifestRow = addvars(manifestRow, double(subStateGroup), "After", "FocStateGroup", ...
    "NewVariableNames", "FocSubStateGroup");
end

function importReport = localEmptyImportReport()
importReport = table( ...
    'Size', [0, 7], ...
    'VariableTypes', {"string", "string", "string", "string", "string", "double", "double"}, ...
    'VariableNames', {"SourceId", "SourceFile", "TestId", "Action", "Reason", ...
    "ClosedLoopRows", "FitEligibleRows"});
end

function row = localImportReportRow(source, action, reason, closedLoopRows)
row = table( ...
    string(source.SourceId), string(source.SourceFile), string(source.TestId), ...
    string(action), string(reason), double(closedLoopRows), double(closedLoopRows), ...
    'VariableNames', {"SourceId", "SourceFile", "TestId", "Action", "Reason", ...
    "ClosedLoopRows", "FitEligibleRows"});
end

function mf4Files = localDiscoverMdfFiles(inputSpec, filePatterns)
inputSpec = string(inputSpec);
if isscalar(inputSpec) && isfolder(inputSpec)
    records = dir(fullfile(inputSpec, "__powerfit_no_matching_file__"));
    for patternIndex = 1:numel(filePatterns)
        records = [records; dir(fullfile(inputSpec, string(filePatterns(patternIndex))))]; %#ok<AGROW>
    end
    if isempty(records)
        error("powerfit:NoMdfFiles", "No MF4 files were found in %s.", inputSpec);
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
