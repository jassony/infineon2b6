function [cfg, stored, hasUnsavedChanges] = sogi_get_parameters
%SOGI_GET_PARAMETERS Read the current SOGI dictionary parameter snapshot.
%   CFG = SOGI_GET_PARAMETERS reads f0, Ts and k without changing the
%   dictionary, loading models or restoring the trial defaults. Existing
%   dictionary connections and unsaved changes remain available to callers.

root = fileparts(mfilename('fullpath'));
dict = Simulink.data.dictionary.open(fullfile(root,'sogi_parameters.sldd'));
connection = onCleanup(@() close(dict)); %#ok<NASGU>
section = getSection(dict,'Design Data');
hasUnsavedChanges=dict.HasUnsavedChanges;
names = {'Cal_SOGI_F0_Hz_f32','Cal_SOGI_Ts_s_f32','Cal_SOGI_K_f32'};
values = zeros(1,3);
for index = 1:numel(names)
    parameter = getValue(getEntry(section,names{index}));
    assert(isa(parameter,'Simulink.Parameter') && isscalar(parameter) && ...
        strcmp(parameter.DataType,'single') && ...
        isa(parameter.Value,'single') && isscalar(parameter.Value) && ...
        isreal(parameter.Value), 'SOGI:DictionaryParameter', ...
        '%s must contain a scalar real single Simulink.Parameter.',names{index});
    values(index) = double(parameter.Value);
end
cfg = sogi_coefficients(values(1),values(2),values(3));
if nargout>1
    expected=sogi_dictionary_values(cfg); names=fieldnames(expected);
    stored=struct;
    for index=1:numel(names)
        parameter=getValue(getEntry(section,names{index}));
        assert(isa(parameter,'Simulink.Parameter') && isscalar(parameter) && ...
            strcmp(parameter.DataType,'single') && isa(parameter.Value,'single') && ...
            isscalar(parameter.Value) && isreal(parameter.Value) && isfinite(parameter.Value), ...
            'SOGI:DictionaryParameter','%s must be a finite scalar single parameter.',names{index});
        stored.(names{index})=parameter.Value;
    end
end
end
