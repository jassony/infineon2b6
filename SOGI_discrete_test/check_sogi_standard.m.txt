function report = check_sogi_standard(reportDir, doCompile)
%CHECK_SOGI_STANDARD Read-only structure, layout and compiled interface checks.
%   REPORT = CHECK_SOGI_STANDARD writes standard_checks.csv and returns a
%   struct. It never edits/saves models or dictionaries. Numerical replay is
%   deliberately separate. Set DOCOMPILE=false for static inspection only;
%   the omitted compiled checks are reported NOT_RUN, never PASS.
arguments
    reportDir (1,:) char = fullfile(fileparts(mfilename('fullpath')),'reports')
    doCompile (1,1) logical = true
end
root=fileparts(mfilename('fullpath'));
models={'sogi_discrete_comparison','sogi_validation'};
wasLoaded=cellfun(@bdIsLoaded,models);
closeGuard=onCleanup(@()closeOwnedModels(models,wasLoaded)); %#ok<NASGU>
rows=cell(0,5); cfg=sogi_get_parameters();
for modelIndex=1:numel(models)
    model=models{modelIndex}; modelFile=fullfile(root,[model '.slx']);
    if ~isfile(modelFile)
        rows(end+1,:)={model,'model_file',modelFile,'FAIL','Required model is missing.'}; %#ok<AGROW>
        continue
    end
    if ~bdIsLoaded(model), load_system(modelFile); end
    dirtyBefore=get_param(model,'Dirty');
    blocks=find_system(model,'FollowLinks','off','LookUnderMasks','all', ...
        'FindAll','on','Type','block');
    % Type=block excludes the root block diagram; keep an explicit guard.
    blocks=blocks(arrayfun(@(h)strcmp(get_param(h,'Type'),'block'),blocks));
    rows=[rows;checkDictionary(model,root)]; %#ok<AGROW>
    rows=[rows;checkParameterization(model)]; %#ok<AGROW>
    rows=[rows;checkPorts(model,blocks)]; %#ok<AGROW>
    rows=[rows;checkLines(model)]; %#ok<AGROW>
    rows=[rows;checkLineGeometry(model,blocks)]; %#ok<AGROW>
    rows=[rows;checkNamesFontsAndOverlap(model,blocks)]; %#ok<AGROW>
    rows=[rows;checkLocalTags(model,blocks)]; %#ok<AGROW>
    if strcmp(model,'sogi_discrete_comparison')
        delays=blocks(arrayfun(@(h)strcmp(get_param(h,'BlockType'),'UnitDelay'),blocks));
        rows(end+1,:)={model,'state_count',model,pass(numel(delays)==8), ...
            sprintf('Unit Delay blocks: %d; exactly 8 required.',numel(delays))}; %#ok<AGROW>
    end
    if doCompile
        rows=[rows;checkCompiled(model,blocks,cfg.Ts)]; %#ok<AGROW>
    else
        rows(end+1,:)={model,'compiled_interfaces',model,'NOT_RUN', ...
            'Compile disabled by caller; types and compiled sample periods were not checked.'}; %#ok<AGROW>
    end
    rows(end+1,:)={model,'no_model_mutation',model, ...
        pass(strcmp(dirtyBefore,get_param(model,'Dirty'))), ...
        'Dirty flag must remain unchanged during inspection.'}; %#ok<AGROW>
end
checks=cell2table(rows,'VariableNames',{'Model','Check','Item','Status','Detail'});
status='PASS';
if any(strcmp(checks.Status,'NOT_RUN')), status='NOT_RUN'; end
if any(strcmp(checks.Status,'FAIL')), status='FAIL'; end
report=struct('status',status,'checks',checks,'compiled',doCompile, ...
    'timestamp',char(datetime('now','Format','yyyy-MM-dd HH:mm:ss')), ...
    'matlabVersion',version,'statusScope','Automatic checks only', ...
    'manualReviewStatus','NOT_RUN', ...
    'manualReviewDetail','Block-name, signal-label and annotation occlusion requires screenshot review; not certified by this checker.');
if ~isfolder(reportDir), mkdir(reportDir); end
writetable(checks,fullfile(reportDir,'standard_checks.csv'),'Encoding','UTF-8');
fprintf('SOGI automatic standard checks: %s (%d PASS, %d FAIL, %d NOT_RUN).\n', ...
    status,sum(strcmp(checks.Status,'PASS')),sum(strcmp(checks.Status,'FAIL')), ...
    sum(strcmp(checks.Status,'NOT_RUN')));
fprintf('Manual review still required: block names, signal labels and annotations must be unobstructed.\n');
end

function rows=checkDictionary(model,root)
rows=cell(0,5); link=get_param(model,'DataDictionary');
[~,name,extension]=fileparts(link);
rows(end+1,:)={model,'dictionary_link',model, ...
    pass(strcmp([name extension],'sogi_parameters.sldd')), ...
    ['Linked dictionary: ' link]};
dictFile=fullfile(root,'sogi_parameters.sldd');
if ~isfile(dictFile)
    rows(end+1,:)={model,'dictionary_base_workspace',dictFile,'FAIL','Dictionary is missing.'};
    return
end
% Open is read-only in this check: no addEntry/setValue/saveChanges call.
% Keep any existing dictionary connection open so user state is preserved.
dict=Simulink.data.dictionary.open(dictFile);
rows(end+1,:)={model,'dictionary_base_workspace',dictFile, ...
    pass(~dict.EnableAccessToBaseWorkspace), ...
    'EnableAccessToBaseWorkspace must be false.'};
end

function rows=checkParameterization(model)
rows=cell(0,5);
try
    sogi_check_parameter_binding(model);
    state='PASS'; detail='Legacy 51 global values are coherent; this does not validate FE physical input signals.';
catch ex
    state='FAIL'; detail=[ex.identifier ': ' ex.message];
end
rows(end+1,:)={model,'parameter_binding',model,state,detail};
guard=get_param(model,'InitFcn');
rows(end+1,:)={model,'parameter_init_guard',model, ...
    pass(contains(guard,'sogi_check_parameter_binding')), ...
    'Legacy compilation guard checks global values; actual FE input validation is separate.'};
guard=get_param(model,'StartFcn');
rows(end+1,:)={model,'parameter_start_guard',model, ...
    pass(contains(guard,'sogi_check_parameter_binding')), ...
    'Start checks active compiled methods after SimulationInput block overrides.'};
scopes={model};
if strcmp(model,'sogi_discrete_comparison')
    scopes=[scopes;find_system(model,'SearchDepth',1,'BlockType','SubSystem')];
end
for i=1:numel(scopes)
    if strcmp(get_param(scopes{i},'Name'),'SOGI_DualOutput_Filter_FE')
        expected={'Input_PU','Reset_States','Cal_SOGI_F0_Hz_f32', ...
            'Cal_SOGI_K_f32','Cal_SOGI_Ts_s_f32'};
        inputs=find_system(scopes{i},'SearchDepth',1,'BlockType','Inport');
        ok=numel(inputs)==5;
        for j=1:numel(inputs)
            port=str2double(get_param(inputs{j},'Port'));
            ok=ok && port>=1 && port<=5 && strcmp(get_param(inputs{j},'Name'),expected{port});
        end
        rows(end+1,:)={model,'fe_physical_input_ports',scopes{i},pass(ok), ...
            'Five formal ports in order: u, reset, f0 [Hz], k [1], Ts [s].'}; %#ok<AGROW>
        rows(end+1,:)={model,'fe_actual_input_validation',scopes{i},'NOT_RUN', ...
            'External actual-input/frozen-parameter guard is not yet implemented or accepted.'}; %#ok<AGROW>
        continue
    end
    hs=find_system(scopes{i},'FindAll','on','SearchDepth',1,'Type','annotation');
    panels=hs(arrayfun(@(h)strcmp(get_param(h,'Tag'),'SOGI_PARAMETERS'),hs));
    ok=numel(panels)==1;
    if ok
        a=get_param(panels,'Object');
        ok=all(contains(a.Text,{'Cal_SOGI_F0_Hz_f32','Cal_SOGI_Ts_s_f32','Cal_SOGI_K_f32'})) ...
            && strcmp(a.ClickFcn,'sogi_parameter_dialog;') && a.FontSize>=14;
    end
    rows(end+1,:)={model,'visible_parameter_entry',scopes{i},pass(ok), ...
        'External/legacy configuration aid only: full physical symbols and at least 14 pt.'}; %#ok<AGROW>
end
end

function rows=checkPorts(model,blocks)
rows=cell(0,5); checked=0;
for n=1:numel(blocks)
    ports=get_param(blocks(n),'PortHandles'); kinds=fieldnames(ports);
    for k=1:numel(kinds)
        handles=ports.(kinds{k});
        for h=reshape(handles,1,[])
            if h<=0, continue; end
            checked=checked+1; line=get_param(h,'Line');
            if isempty(line) || all(line<=0)
                rows(end+1,:)={model,'connected_ports',getfullname(blocks(n)), ...
                    'FAIL',sprintf('Unconnected %s port %s.',kinds{k},num2str(get_param(h,'PortNumber')))}; %#ok<AGROW>
            end
        end
    end
end
if isempty(rows)
    rows={model,'connected_ports',model,'PASS',sprintf('All %d discovered ports are connected.',checked)};
end
end

function rows=checkLines(model)
rows=cell(0,5);
lines=find_system(model,'FollowLinks','off','LookUnderMasks','all', ...
    'FindAll','on','Type','line');
for h=reshape(lines,1,[])
    source=get_param(h,'SrcPortHandle'); ancestor=h;
    while all(source<=0)
        parent=get_param(ancestor,'LineParent');
        if isempty(parent) || parent<=0, break; end
        ancestor=parent; source=get_param(ancestor,'SrcPortHandle');
    end
    destinations=get_param(h,'DstPortHandle');
    children=get_param(h,'LineChildren');
    hasSource=~isempty(source) && any(source>0);
    % A trunk may have only child branches; each leaf needs an endpoint.
    leaf=isempty(children) || all(children<=0);
    hasDestination=~leaf || (~isempty(destinations) && all(destinations>0));
    if ~hasSource || ~hasDestination
        rows(end+1,:)={model,'dangling_lines',sprintf('line %.17g',h),'FAIL', ...
            sprintf('Valid source: %d; valid leaf destination: %d.',hasSource,hasDestination)}; %#ok<AGROW>
    end
end
if isempty(rows)
    rows={model,'dangling_lines',model,'PASS',sprintf('%d line segments checked.',numel(lines))};
end
end

function rows=checkLineGeometry(model,blocks)
rows=cell(0,5); diagonalCount=0; obstructionCount=0; segmentCount=0;
lines=find_system(model,'FollowLinks','off','LookUnderMasks','all', ...
    'FindAll','on','Type','line');
parents=cell(numel(blocks),1); positions=zeros(numel(blocks),4);
paths=cell(numel(blocks),1);
for n=1:numel(blocks)
    parents{n}=get_param(blocks(n),'Parent');
    positions(n,:)=get_param(blocks(n),'Position');
    paths{n}=getfullname(blocks(n));
end
for h=reshape(lines,1,[])
    points=get_param(h,'Points');
    if isempty(points) || size(points,1)<2, continue; end
    parent=get_param(h,'Parent');
    siblings=find(strcmp(parents,parent));
    endpointBlocks=lineEndpointBlocks(h);
    unrelated=siblings(~ismember(paths(siblings),endpointBlocks));
    lineLabel=sprintf('line %.17g',h);
    for segment=1:size(points,1)-1
        a=points(segment,:); b=points(segment+1,:); delta=b-a;
        if all(delta==0), continue; end
        segmentCount=segmentCount+1;
        if delta(1)~=0 && delta(2)~=0
            diagonalCount=diagonalCount+1;
            rows(end+1,:)={model,'orthogonal_line_segments',lineLabel,'FAIL', ...
                sprintf('Scope %s; segment %d is diagonal: [%g %g] -> [%g %g].', ...
                parent,segment,a(1),a(2),b(1),b(2))}; %#ok<AGROW>
        end
        for target=reshape(unrelated,1,[])
            if entersStrictRectangle(a,b,positions(target,:))
                obstructionCount=obstructionCount+1;
                rows(end+1,:)={model,'line_block_obstruction',lineLabel,'FAIL', ...
                    sprintf('Scope %s; segment %d [%g %g] -> [%g %g] crosses interior of %s.', ...
                    parent,segment,a(1),a(2),b(1),b(2),paths{target})}; %#ok<AGROW>
            end
        end
    end
end
rows(end+1,:)={model,'orthogonal_line_summary',model,pass(diagonalCount==0), ...
    sprintf('%d nonzero segments checked; %d diagonal segments. Zero-length segments ignored.', ...
    segmentCount,diagonalCount)};
rows(end+1,:)={model,'line_block_summary',model,pass(obstructionCount==0), ...
    sprintf('%d segment/block interior intersections. Only actual source/destination blocks excluded; labels and annotations require manual review.', ...
    obstructionCount)};
end

function paths=lineEndpointBlocks(line)
% Branches can have an absent local source handle; walk to the ancestor
% source, then collect destination ports from this branch and its children.
paths=cell(0,1); ancestor=line; source=get_param(ancestor,'SrcPortHandle');
while isempty(source) || all(source<=0)
    parent=get_param(ancestor,'LineParent');
    if isempty(parent) || parent<=0, break; end
    ancestor=parent; source=get_param(ancestor,'SrcPortHandle');
end
for h=reshape(source,1,[])
    if h>0, paths{end+1,1}=get_param(h,'Parent'); end %#ok<AGROW>
end
pending=line;
while ~isempty(pending)
    current=pending(end); pending(end)=[];
    destinations=get_param(current,'DstPortHandle');
    for h=reshape(destinations,1,[])
        if h>0, paths{end+1,1}=get_param(h,'Parent'); end %#ok<AGROW>
    end
    children=get_param(current,'LineChildren');
    pending=[pending,reshape(children(children>0),1,[])]; %#ok<AGROW>
end
paths=unique(paths);
end

function intersects=entersStrictRectangle(a,b,rectangle)
% Clip a segment against the open rectangle. Boundary-only contact (including
% a single corner touch) is allowed; any positive interval inside is not.
delta=b-a; first=0; last=1;
for axis=1:2
    low=rectangle(axis); high=rectangle(axis+2);
    if delta(axis)==0
        if a(axis)<=low || a(axis)>=high, intersects=false; return; end
    else
        interval=sort(([low high]-a(axis))/delta(axis));
        first=max(first,interval(1)); last=min(last,interval(2));
        if first>=last, intersects=false; return; end
    end
end
intersects=first<last;
end

function rows=checkNamesFontsAndOverlap(model,blocks)
rows=cell(0,5); fontFailures=0; nameFailures=0; overlapFailures=0;
parents=cell(numel(blocks),1); positions=zeros(numel(blocks),4);
for n=1:numel(blocks)
    path=getfullname(blocks(n)); name=get_param(blocks(n),'Name');
    valid=~isempty(regexp(name,'^[A-Za-z][A-Za-z0-9_]*$','once')) && ...
        ~contains(name,'__') && ~endsWith(name,'_');
    if ~valid
        nameFailures=nameFailures+1;
        rows(end+1,:)={model,'block_names',path,'FAIL', ...
            'Use letters/digits/single underscores; start with a letter; no trailing underscore.'}; %#ok<AGROW>
    end
    font=get_param(blocks(n),'FontSize');
    if ischar(font) || isstring(font), font=str2double(font); end
    if ~isscalar(font) || ~isfinite(font) || font<12
        fontFailures=fontFailures+1;
        rows(end+1,:)={model,'font_minimum',path,'FAIL', ...
            sprintf('FontSize=%g; explicit FontSize >=12 required for readable labels.',font)}; %#ok<AGROW>
    end
    parents{n}=get_param(blocks(n),'Parent');
    positions(n,:)=get_param(blocks(n),'Position');
end
for n=1:numel(blocks)
    siblings=find(strcmp(parents,parents{n})); siblings=siblings(siblings>n);
    a=positions(n,:);
    for other=reshape(siblings,1,[])
        b=positions(other,:);
        width=min(a(3),b(3))-max(a(1),b(1));
        height=min(a(4),b(4))-max(a(2),b(2));
        if width>0 && height>0
            overlapFailures=overlapFailures+1;
            rows(end+1,:)={model,'block_overlap',getfullname(blocks(n)), ...
                'FAIL',sprintf('Overlaps %s by %g x %g pixels.',getfullname(blocks(other)),width,height)}; %#ok<AGROW>
        end
    end
end
if nameFailures==0, rows(end+1,:)={model,'block_names',model,'PASS','All block names follow the naming syntax.'}; end
if fontFailures==0, rows(end+1,:)={model,'font_minimum',model,'PASS','All block labels use FontSize >=12.'}; end
if overlapFailures==0
    rows(end+1,:)={model,'block_overlap',model,'PASS', ...
        'No positive-area overlap between sibling block rectangles; annotations are excluded.'};
end
end

function rows=checkLocalTags(model,blocks)
rows=cell(0,5);
gotos=blocks(arrayfun(@(h)strcmp(get_param(h,'BlockType'),'Goto'),blocks));
froms=blocks(arrayfun(@(h)strcmp(get_param(h,'BlockType'),'From'),blocks));
for h=reshape(gotos,1,[])
    tag=get_param(h,'GotoTag'); parent=get_param(h,'Parent');
    candidates=gotos(arrayfun(@(x)strcmp(get_param(x,'Parent'),parent) && ...
        strcmp(get_param(x,'GotoTag'),tag),gotos));
    visibility=get_param(h,'TagVisibility');
    valid=strcmp(visibility,'local') && numel(candidates)==1;
    rows(end+1,:)={model,'local_goto_unique',getfullname(h),pass(valid), ...
        sprintf('Tag %s; visibility %s; %d producer(s) in this scope.',tag,visibility,numel(candidates))}; %#ok<AGROW>
end
for h=reshape(froms,1,[])
    tag=get_param(h,'GotoTag'); parent=get_param(h,'Parent');
    candidates=gotos(arrayfun(@(x)strcmp(get_param(x,'Parent'),parent) && ...
        strcmp(get_param(x,'GotoTag'),tag) && strcmp(get_param(x,'TagVisibility'),'local'),gotos));
    rows(end+1,:)={model,'local_from_resolves',getfullname(h),pass(numel(candidates)==1), ...
        sprintf('Tag %s resolves to %d local producer(s) in the same scope.',tag,numel(candidates))}; %#ok<AGROW>
end
if isempty(gotos) && isempty(froms)
    rows={model,'local_tag_routing',model,'PASS','No Goto/From blocks used.'};
end
end

function rows=checkCompiled(model,blocks,Ts)
rows=cell(0,5);
if ~strcmp(get_param(model,'SimulationStatus'),'stopped')
    rows={model,'compiled_interfaces',model,'NOT_RUN','Model is active; compilation would interrupt user work.'};
    return
end
try
    % Install cleanup before compile so a failed partial compile is terminated.
    compileGuard=onCleanup(@()terminateCompile(model)); %#ok<NASGU>
    feval(model,[],[],[],'compile');
    rows(end+1,:)={model,'compile',model,'PASS','Model compiled successfully.'};
    isAlgorithm=strcmp(model,'sogi_discrete_comparison');
    for n=1:numel(blocks)
        kind=get_param(blocks(n),'BlockType');
        rootPort=strcmp(get_param(blocks(n),'Parent'),model) && any(strcmp(kind,{'Inport','Outport'}));
        state=strcmp(kind,'UnitDelay');
        if ~(rootPort || state), continue; end
        if ~isAlgorithm && ~rootPort, continue; end
        path=getfullname(blocks(n)); ports=get_param(blocks(n),'PortHandles');
        if strcmp(kind,'Outport'), selected=ports.Inport; else, selected=ports.Outport; end
        expectedType='double';
        if isAlgorithm
            expectedType='single';
            if strcmp(kind,'Inport') && strcmp(get_param(blocks(n),'Port'),'2'), expectedType='uint8'; end
        end
        for h=reshape(selected,1,[])
            actualType=get_param(h,'CompiledPortDataType');
            rows(end+1,:)={model,'compiled_type',path,pass(strcmp(actualType,expectedType)), ...
                sprintf('Actual %s; required %s.',actualType,expectedType)}; %#ok<AGROW>
        end
        sample=get_param(blocks(n),'CompiledSampleTime');
        if iscell(sample), sample=vertcat(sample{:}); end
        valid=isnumeric(sample) && size(sample,2)==2 && ...
            all(abs(sample(:,1)-Ts)<=max(1e-12,Ts*1e-6)) && all(sample(:,2)==0);
        rows(end+1,:)={model,'compiled_sample_time',path,pass(valid), ...
            sprintf('Actual %s; required [%0.17g 0].',mat2str(sample,17),Ts)}; %#ok<AGROW>
    end
catch exception
    rows(end+1,:)={model,'compiled_interfaces',model,'FAIL', ...
        sprintf('%s: %s',exception.identifier,exception.message)};
end
end

function terminateCompile(model)
if bdIsLoaded(model)
    try
        feval(model,[],[],[],'term');
    catch exception
        warning('SOGI:CompileCleanup','Compile termination for %s: %s',model,exception.message);
    end
end
end

function closeOwnedModels(models,wasLoaded)
for n=numel(models):-1:1
    if ~wasLoaded(n) && bdIsLoaded(models{n}) && strcmp(get_param(models{n},'SimulationStatus'),'stopped')
        close_system(models{n},0);
    end
end
end

function status=pass(valid)
status='FAIL'; if valid, status='PASS'; end
end
