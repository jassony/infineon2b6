function [powerfitDatabase, importReport] = powerfit_update_database(mf4Files, cfg)
%POWERFIT_UPDATE_DATABASE Append unseen MF4 logs to the local MAT database.
%
% A SHA-256 source fingerprint is the duplicate key.  Existing rows are never
% replaced or deleted by this function; a changed or re-recorded log has a new
% fingerprint and remains separately traceable.

if nargin < 2 || isempty(cfg)
    cfg = powerfit_default_config();
end

mf4Files = localNormalizeFiles(mf4Files);
if isempty(mf4Files)
    error("powerfit:NoFiles", "No MF4 files were supplied for import.");
end

if isfile(cfg.DatabaseFile)
    loaded = load(cfg.DatabaseFile, "powerfitDatabase");
    if ~isfield(loaded, "powerfitDatabase")
        error("powerfit:DatabaseSchema", "Database file does not contain powerfitDatabase.");
    end
    powerfitDatabase = loaded.powerfitDatabase;
    localValidateDatabase(powerfitDatabase, cfg.SchemaVersion);
else
    powerfitDatabase = powerfit_empty_database();
end

importReport = localEmptyImportReport();
databaseChanged = false;
for fileIndex = 1:numel(mf4Files)
    source = powerfit_source_record(mf4Files(fileIndex));
    existingSource = any(powerfitDatabase.Manifest.SourceId == source.SourceId ...
        & powerfitDatabase.Manifest.FitGroupId == string(cfg.Profile.FitGroupId));
    if existingSource
        reportRow = localImportReportRow(source, "SkippedDuplicate", ...
            "SHA-256 fingerprint already exists in this fit group.", 0, 0);
        importReport = [importReport; reportRow]; %#ok<AGROW>
        continue
    end

    [newSamples, manifestRow] = powerfit_extract_mf4(source.SourceFile, cfg);
    powerfitDatabase.Manifest = [powerfitDatabase.Manifest; manifestRow];
    if startsWith(manifestRow.Status, "Imported")
        powerfitDatabase.Samples = [powerfitDatabase.Samples; newSamples];
    end

    reportRow = localImportReportRow(source, manifestRow.Status, manifestRow.Reason, ...
        height(newSamples), sum(newSamples.FitEligible));
    importReport = [importReport; reportRow]; %#ok<AGROW>
    databaseChanged = true;

    if cfg.PrintProgress
        fprintf("%s: %s (%d aligned rows, %d fit-eligible)\n", ...
            manifestRow.Status, source.TestId, height(newSamples), sum(newSamples.FitEligible));
    end
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

function files = localNormalizeFiles(mf4Files)
if ischar(mf4Files) || isstring(mf4Files)
    files = string(mf4Files);
elseif iscellstr(mf4Files)
    files = string(mf4Files);
else
    error("powerfit:InvalidFileList", "mf4Files must be a string, character vector, or cell array of paths.");
end
files = files(:);
files = files(strlength(files) > 0);
end

function localValidateDatabase(powerfitDatabase, expectedSchemaVersion)
requiredFields = ["SchemaVersion", "Samples", "Manifest"];
if ~all(isfield(powerfitDatabase, requiredFields))
    error("powerfit:DatabaseSchema", "The database schema is incomplete.");
end
if string(powerfitDatabase.SchemaVersion) ~= string(expectedSchemaVersion)
    error("powerfit:DatabaseSchema", ...
        "Database schema version %s is not compatible with expected version %s.", ...
        string(powerfitDatabase.SchemaVersion), string(expectedSchemaVersion));
end

template = powerfit_empty_database();
if ~isequal(powerfitDatabase.Samples.Properties.VariableNames, ...
        template.Samples.Properties.VariableNames) ...
        || ~isequal(powerfitDatabase.Manifest.Properties.VariableNames, ...
        template.Manifest.Properties.VariableNames)
    error("powerfit:DatabaseSchema", "Database table variables do not match this workflow version.");
end
end

function importReport = localEmptyImportReport()
importReport = table( ...
    'Size', [0, 7], ...
    'VariableTypes', {'string', 'string', 'string', 'string', 'string', 'double', 'double'}, ...
    'VariableNames', {'SourceId', 'SourceFile', 'TestId', 'Action', 'Reason', ...
    'AlignedRows', 'FitEligibleRows'});
end

function row = localImportReportRow(source, action, reason, alignedRows, eligibleRows)
row = table( ...
    string(source.SourceId), string(source.SourceFile), string(source.TestId), ...
    string(action), string(reason), double(alignedRows), double(eligibleRows), ...
    'VariableNames', {'SourceId', 'SourceFile', 'TestId', 'Action', 'Reason', ...
    'AlignedRows', 'FitEligibleRows'});
end
