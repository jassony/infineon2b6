function cfg = sogi_set_parameters(f0, Ts, k)
%SOGI_SET_PARAMETERS Save a coherent SOGI snapshot before simulation.
%   CFG = SOGI_SET_PARAMETERS(F0,TS,K) updates the three base parameters
%   and all 48 derived coefficients in sogi_parameters.sldd. Only Value
%   changes; units, ranges, descriptions and storage classes are preserved.
%   Both SOGI models must be stopped with Fast Restart off. Existing
%   unsaved dictionary edits must be saved or discarded by their owner first.
arguments
    f0 (1,1) {mustBeNumeric,mustBeReal,mustBeFinite,mustBePositive}
    Ts (1,1) {mustBeNumeric,mustBeReal,mustBeFinite,mustBePositive}
    k (1,1) {mustBeNumeric,mustBeReal,mustBeFinite,mustBePositive}
end

assertModelsStopped;
cfg = sogi_coefficients(double(f0),double(Ts),double(k));
for index = 1:numel(cfg.methods)
    method = cfg.methods(index);
    assert(method.stable, 'SOGI:UnstableParameters', ...
        '%s is unstable: double pole radius %.9g, single pole radius %.9g.', ...
        cfg.names{index},method.radius,method.singleRadius);
end
values = sogi_dictionary_values(cfg);
names = fieldnames(values);
assert(numel(names)==51,'SOGI:SnapshotContract', ...
    'Expected 3 base parameters and 48 derived coefficients.');

root = fileparts(mfilename('fullpath'));
dict = Simulink.data.dictionary.open(fullfile(root,'sogi_parameters.sldd'));
connection = onCleanup(@() close(dict)); %#ok<NASGU>
assertSaved(dict);
section = getSection(dict,'Design Data');
entries = cell(size(names));
before = cell(size(names));
after = cell(size(names));

% Prepare every entry before changing any dictionary value. Parameter
% objects are handles, so copy each object to keep rollback data separate.
for index = 1:numel(names)
    name = names{index};
    entries{index} = getEntry(section,name);
    parameter = getValue(entries{index});
    assert(isa(parameter,'Simulink.Parameter') && isscalar(parameter) && ...
        strcmp(parameter.DataType,'single') && ...
        isa(parameter.Value,'single') && isscalar(parameter.Value) && ...
        isreal(parameter.Value), 'SOGI:DictionaryParameter', ...
        '%s must contain a scalar real single Simulink.Parameter.',name);
    assert(isa(values.(name),'single') && isscalar(values.(name)) && ...
        isreal(values.(name)) && isfinite(values.(name)), ...
        'SOGI:SnapshotContract','%s is not a finite scalar single value.',name);
    [~,source,extension] = fileparts(entries{index}.DataSource);
    assert(strcmpi([source extension],'sogi_parameters.sldd'), ...
        'SOGI:DictionaryParameter', ...
        '%s must be owned by the local sogi_parameters.sldd dictionary.',name);
    before{index} = copy(parameter);
    after{index} = copy(parameter);
    after{index}.Value = values.(name);
end

try
    for index = 1:numel(names)
        setValue(entries{index},after{index});
    end
    saveChanges(dict);
catch originalFailure
    % Restore the complete prior snapshot, including when saveChanges fails.
    % No unrelated entry was edited and the dictionary was clean on entry.
    rollbackFailures = {};
    for index = 1:numel(names)
        try
            setValue(entries{index},before{index});
        catch rollbackFailure
            rollbackFailures{end+1} = rollbackFailure; %#ok<AGROW>
        end
    end
    if isempty(rollbackFailures)
        try
            saveChanges(dict);
        catch rollbackFailure
            rollbackFailures{end+1} = rollbackFailure;
        end
    end
    if ~isempty(rollbackFailures)
        failure = MException('SOGI:ParameterRollbackFailed', ...
            ['Parameter update failed and the previous saved snapshot could ' ...
             'not be confirmed. Inspect sogi_parameters.sldd before simulation.']);
        failure = addCause(failure,originalFailure);
        for index = 1:numel(rollbackFailures)
            failure = addCause(failure,rollbackFailures{index});
        end
        throw(failure);
    end
    rethrow(originalFailure);
end
end

function assertModelsStopped
models = {'sogi_discrete_comparison','sogi_validation'};
for index = 1:numel(models)
    model = models{index};
    if bdIsLoaded(model)
        assert(strcmp(get_param(model,'SimulationStatus'),'stopped'), ...
            'SOGI:ModelRunning','Stop %s before applying SOGI parameters.',model);
        assert(strcmp(get_param(model,'FastRestart'),'off'), ...
            'SOGI:FastRestart', ...
            'Turn Fast Restart off for %s before applying SOGI parameters.',model);
    end
end
end

function assertSaved(dict)
assert(~dict.HasUnsavedChanges,'SOGI:UnsavedDictionary', ...
    ['%s contains unsaved changes. Save or discard those changes explicitly ' ...
     'before applying SOGI parameters.'],filepath(dict));
% saveChanges also saves referenced dictionaries. Preserve their pending
% edits too, although the delivered SOGI dictionary has no references.
for index = 1:numel(dict.DataSources)
    reference = Simulink.data.dictionary.open(dict.DataSources{index});
    connection = onCleanup(@() close(reference));
    assertSaved(reference);
    clear connection
end
end
