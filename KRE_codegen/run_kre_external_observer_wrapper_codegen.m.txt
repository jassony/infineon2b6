function generatedDir = run_kre_external_observer_wrapper_codegen()
%RUN_KRE_EXTERNAL_OBSERVER_WRAPPER_CODEGEN Build the KRE observer with ERT.
%   The script adds the protected reference observer to the MATLAB path and
%   emits generated sources beside this script. No calibration or XCP files
%   are produced.

thisDir = fileparts(mfilename('fullpath'));
projectDir = fileparts(thisDir);
modelFile = fullfile(thisDir, 'kre_external_observer_wrapper.slx');

addpath(thisDir);
addpath(fullfile(projectDir, 'ges', 'SensorlessFocFOSMOExample'));

load_system(modelFile);
cleanupObj = onCleanup(@() close_system('kre_external_observer_wrapper', 0)); %#ok<NASGU>

% Keep regeneration aligned with the CM4 firmware target.
configSet = getActiveConfigSet('kre_external_observer_wrapper');
configSet.set_param('SystemTargetFile', 'ert.tlc');
configSet.set_param('TargetLang', 'C');
configSet.set_param('ProdHWDeviceType', 'ARM Compatible->ARM Cortex-M');
configSet.set_param('Solver', 'FixedStepDiscrete');
configSet.set_param('FixedStep', '5e-5');
save_system('kre_external_observer_wrapper', modelFile);

oldDir = pwd;
cd(thisDir);
dirCleanup = onCleanup(@() cd(oldDir)); %#ok<NASGU>

slbuild('kre_external_observer_wrapper');
generatedDir = fullfile(thisDir, 'kre_external_observer_wrapper_ert_rtw');
if ~isfolder(generatedDir)
    error('KRE:CodegenOutputMissing', ...
        'ERT build completed without creating %s.', generatedDir);
end

% R2026a can emit a generic atan2f helper with external linkage for each
% generated model. Give the KRE copy a unique, repeatable symbol name.
sourceFile = fullfile(generatedDir, 'kre_external_observer_wrapper.c');
sourceText = fileread(sourceFile);
sourceText = strrep(sourceText, 'kre_kre_rt_atan2f_snf', ...
    'kre_rt_atan2f_snf');
sourceText = strrep(sourceText, 'real32_T rt_atan2f_snf', ...
    'real32_T kre_rt_atan2f_snf');
sourceText = strrep(sourceText, '= rt_atan2f_snf', ...
    '= kre_rt_atan2f_snf');
sourceText = strrep(sourceText, 'kre_kre_rt_atan2f_snf', ...
    'kre_rt_atan2f_snf');
fileId = fopen(sourceFile, 'w');
if fileId < 0
    error('KRE:CodegenOutputWriteFailed', ...
        'Unable to update generated source %s.', sourceFile);
end
fwrite(fileId, sourceText, 'char');
fclose(fileId);

% Keep the private declaration synchronized with the renamed helper. The
% ordered replacements make this operation idempotent when slbuild reuses a
% current cache instead of regenerating the C source.
privateHeader = fullfile(generatedDir, ...
    'kre_external_observer_wrapper_private.h');
privateText = fileread(privateHeader);
privateText = strrep(privateText, 'kre_kre_rt_atan2f_snf', ...
    'kre_rt_atan2f_snf');
privateText = strrep(privateText, 'rt_atan2f_snf', ...
    'kre_rt_atan2f_snf');
privateText = strrep(privateText, 'kre_kre_rt_atan2f_snf', ...
    'kre_rt_atan2f_snf');
fileId = fopen(privateHeader, 'w');
if fileId < 0
    error('KRE:CodegenPrivateHeaderWriteFailed', ...
        'Unable to update generated private header %s.', privateHeader);
end
fwrite(fileId, privateText, 'char');
fclose(fileId);
end
