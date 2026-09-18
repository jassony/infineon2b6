function build_id_map_reference_wrapper()
%BUILD_ID_MAP_REFERENCE_WRAPPER Generate ERT C from the zero-default table.

thisFolder = fileparts(mfilename('fullpath'));
modelName = 'id_map_reference_wrapper';
modelFile = fullfile(thisFolder, modelName + ".slx");

addpath(thisFolder);
open_system(modelFile);
id_map_reference_install_defaults(modelName);
save_system(modelName);

originalFolder = pwd;
cleanup = onCleanup(@() cd(originalFolder));
cd(thisFolder);
slbuild(modelName);
localRemoveTransientArtifacts(thisFolder, modelName);
end

function localRemoveTransientArtifacts(thisFolder, modelName)
rtwFolder = fullfile(thisFolder, modelName + "_ert_rtw");
cacheFiles = {fullfile(thisFolder, modelName + ".slxc"), ...
    fullfile(rtwFolder, 'buildInfo.mat'), ...
    fullfile(rtwFolder, 'codeInfo.mat'), ...
    fullfile(rtwFolder, 'codedescriptor.dmr'), ...
    fullfile(rtwFolder, 'rtw_proj.tmw'), ...
    fullfile(rtwFolder, 'rtwtypeschksum.mat')};

for index = 1:numel(cacheFiles)
    if isfile(cacheFiles{index})
        delete(cacheFiles{index});
    end
end

cacheFolders = {fullfile(thisFolder, 'slprj'), fullfile(rtwFolder, 'tmwinternal')};
for index = 1:numel(cacheFolders)
    if isfolder(cacheFolders{index})
        rmdir(cacheFolders{index}, 's');
    end
end
end
