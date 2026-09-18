function powerfit_closedloop_initialize_database(cfg)
%POWERFIT_CLOSEDLOOP_INITIALIZE_DATABASE Create the isolated closed-loop DB.
%
% This only creates a missing database.  It never replaces an existing one.

if isfile(cfg.DatabaseFile)
    return
end

powerfitDatabase = powerfit_empty_database();
powerfitDatabase.SchemaVersion = "1.1-closedloop";
powerfitDatabase.Samples.FocSubState = nan(height(powerfitDatabase.Samples), 1);
powerfitDatabase.Samples = movevars(powerfitDatabase.Samples, "FocSubState", ...
    "After", "FocState");
powerfitDatabase.Manifest.FocSubStateGroup = nan(height(powerfitDatabase.Manifest), 1);
powerfitDatabase.Manifest = movevars(powerfitDatabase.Manifest, "FocSubStateGroup", ...
    "After", "FocStateGroup");

databaseFolder = fileparts(cfg.DatabaseFile);
if ~isfolder(databaseFolder)
    mkdir(databaseFolder);
end
save(char(cfg.DatabaseFile), "powerfitDatabase", "-v7.3");
end
