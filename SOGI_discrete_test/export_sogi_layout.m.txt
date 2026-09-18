function files = export_sogi_layout(out)
%EXPORT_SOGI_LAYOUT Export every scope for the separate visual review gate.
% This does not certify readability or save/change the model topology.
root = fileparts(mfilename('fullpath'));
if nargin==0; out = fullfile(root,'reports'); end
if ~isfolder(out); mkdir(out); end
models = {'sogi_discrete_comparison','sogi_validation'};
for i=1:numel(models)
    if ~bdIsLoaded(models{i}); open_system(fullfile(root,[models{i} '.slx'])); end
end
scopes = {models{1}};
names = {'model_layout'};
for method = {'FE','BE','Tustin','ZOH'}
    scopes{end+1} = sogi_method_block(models{1},method{1}); %#ok<AGROW>
    names{end+1} = [lower(method{1}) '_layout']; %#ok<AGROW>
end
b = find_system(models{2},'SearchDepth',1,'Type','Block','Name','Continuous_Reference');
assert(numel(b)==1,'SOGI:Export','Continuous scope must be unique.');
scopes = [scopes,{models{2},Simulink.ID.getFullName(Simulink.ID.getSID(b{1}))}];
names = [names,{'validation_layout','continuous_layout'}];
zoom = cellfun(@(s)get_param(s,'ZoomFactor'),scopes,'UniformOutput',false);
cleanup = onCleanup(@()restoreZoom(scopes,zoom)); %#ok<NASGU>
files = strings(numel(scopes),1);
for i=1:numel(scopes)
    files(i) = fullfile(out,[names{i} '.png']);
    print(['-s' scopes{i}],'-dpng','-r140',char(files(i)));
end
end

function restoreZoom(scopes,zoom)
for i=1:numel(scopes)
    set_param(scopes{i},'ZoomFactor',zoom{i});
end
end
