function sogi_apply_fe_input_layout(scopeMode)
%SOGI_APPLY_FE_INPUT_LAYOUT Lay out the FE physical-parameter input trial.
%   Requires the already constructed model. Changes only diagram geometry,
%   display properties and annotations; never creates/deletes connections,
%   blocks or model parameters and never saves a model.
%   Use ('internal') to adjust only the FE internals and preserve root layout.

if nargin==0; scopeMode='all'; end
assert(any(strcmp(scopeMode,{'all','internal'})),'SOGI:LayoutScope','Use all or internal.');
model = 'sogi_discrete_comparison';
assert(bdIsLoaded(model),'SOGI:LayoutModel','Load %s before laying it out.',model);
fe = scopeByName(model,'SOGI_DualOutput_Filter_FE');
styleScope(fe);

% Parameters/reset conditioning are on the left. D/Q updates align in rows;
% output readout sits immediately to the right of each state publisher.
positions = {
    'Cal_SOGI_F0_Hz_f32',       [90 170 120 190];
    'Cal_SOGI_Ts_s_f32',        [90 280 120 300];
    'Cal_SOGI_K_f32',           [90 390 120 410];
    'Frequency_To_Radps',      [300 150 420 210];
    'Step_Angle',              [555 180 675 240];
    'Publish_Step_Angle',      [745 195 875 225];
    'Publish_SOGI_Gain',       [430 385 560 415];
    'Input_PU',                [90 650 120 670];
    'Reset_States',            [90 780 120 800];
    'Single_Zero',             [50 875 170 925];
    'Publish_Reset',           [280 775 410 805];
    'Publish_Zero',            [280 885 410 915];
    'Zero_For_Input_Reset',    [260 480 390 510];
    'Reset_For_Input',         [260 555 390 585];
    'Reset_Input_PU',          [480 530 600 610];
    'Publish_Input_PU',        [640 555 770 585];
    'InPhase_For_Output',      [3140 635 3270 665];
    'InPhase_D_PU',            [3400 640 3430 660];
    'Quadrature_For_Output',   [3140 1015 3270 1045];
    'Quadrature_Q_PU',         [3400 1020 3430 1040];
    'Input_For_Error',         [810 605 930 635];
    'InPhase_For_Error',       [810 700 930 730];
    'Input_Minus_InPhase',     [1000 620 1060 680];
    'Gain_For_Damping',        [1080 520 1210 550];
    'Damped_Input_Error',      [1260 620 1380 680];
    'Quadrature_For_Correction',[1330 730 1460 760];
    'Subtract_Quadrature',     [1520 620 1580 680];
    'Step_For_InPhase',        [1580 520 1710 550];
    'InPhase_Increment',       [1760 620 1880 680];
    'InPhase_For_Update',      [1810 730 1940 760];
    'Update_InPhase_State',    [2020 620 2080 680];
    'Delay_InPhase_State',     [2250 620 2330 680];
    'Zero_For_InPhase_Reset',  [2350 455 2480 485];
    'Reset_For_InPhase',       [2350 520 2480 550];
    'Reset_InPhase_State',     [2590 610 2710 690];
    'Publish_InPhase_State',   [2970 635 3100 665];
    'InPhase_For_Quadrature',  [1580 960 1710 990];
    'Step_For_Quadrature',     [1580 1090 1710 1120];
    'Quadrature_Increment',    [1760 1000 1880 1060];
    'Quadrature_For_Update',   [1810 1110 1940 1140];
    'Update_Quadrature_State', [2020 1000 2080 1060];
    'Delay_Quadrature_State',  [2250 1000 2330 1060];
    'Zero_For_Quadrature_Reset',[2350 835 2480 865];
    'Reset_For_Quadrature',    [2350 900 2480 930];
    'Reset_Quadrature_State',  [2590 990 2710 1070];
    'Publish_Quadrature_State',[2970 1015 3100 1045]};
for index = 1:size(positions,1)
    place(fe,positions{index,1},positions{index,2});
end
note(fe,'SOGI_FE_INPUT_EQUATIONS',[50 -20],sprintf([ ...
    'FE | physical parameter inputs | validation status: NOT_RUN\n', ...
    'Continuous: dD/dt = k*w0*(u-D) - w0*Q; dQ/dt = w0*D; w0 = 2*pi*f0.\n', ...
    'D_next = D + Ts*w0*(k*(u-D)-Q); Q_next = Q + Ts*w0*D.\n', ...
    'Inputs f0 [Hz], k [1], Ts [s] enter this subsystem directly. ', ...
    'Ts must match the compiled execution period.']));
note(fe,'SOGI_FE_INPUT_OUTPUTS',[2880 430],sprintf([ ...
    'D[k], Q[k]: current outputs\n', ...
    'Read reset-masked states.']));
note(fe,'SOGI_FE_INPUT_STATES',[810 1190],sprintf([ ...
    'D update (upper row), Q update (lower row); Unit Delays store next states.\n', ...
    'X1_k = D[k]; X2_k = Q[k]; U_k = reset-masked input.\n', ...
    'Local tags return current states. Reset clears effective input, states and outputs.']));
set_param(fe,'ContentPreviewEnabled','off','ZoomFactor','100');
routeScope(fe);
% Upper reset sources must use distinct nested lanes: a shared vertical
% segment would visually join zero and reset, although the nets are separate.
for resetBlock = {'Reset_InPhase_State','Reset_Quadrature_State'}
    block = scopeByName(fe,resetBlock{1});
    ports = get_param(block,'PortHandles');
    for input = 1:2
        line = get_param(ports.Inport(input),'Line');
        source = get_param(get_param(line,'SrcPortHandle'),'Position');
        target = get_param(ports.Inport(input),'Position');
        lane = target(1)-35-60*(input-1);
        set_param(line,'Points',[source;lane source(2);lane target(2);target]);
    end
end

if strcmp(scopeMode,'internal'); return; end
styleScope(model);
place(model,'Meas_SOGI_In_PU_f32',[220 140 250 160]);
place(model,'Meas_SOGI_Rst_u8',[220 390 250 410]);
place(model,'Publish_Input',[1000 125 1280 175]);
place(model,'Publish_Reset',[1000 375 1280 425]);
rootParameters = {
    'Cal_SOGI_F0_Hz_f32','Publish_FE_F0',-790;
    'Cal_SOGI_K_f32','Publish_FE_K',-550;
    'Cal_SOGI_Ts_s_f32','Publish_FE_Ts',-310};
for index = 1:size(rootParameters,1)
    y = rootParameters{index,3};
    place(model,rootParameters{index,1},[220 y-10 250 y+10]);
    place(model,rootParameters{index,2},[1000 y-25 1280 y+25]);
end
methods = {'FE','BE','Tustin','ZOH'};
methodBlocks = {'SOGI_DualOutput_Filter_FE','BE','Tustin','ZOH'};
tops = [780 1950 3120 4290];
for index = 1:4
    top = tops(index);
    block = place(model,methodBlocks{index},[2870 top 3770 top+600]);
    set_param(block,'ContentPreviewEnabled','off');
    ports = get_param(block,'PortHandles');
    sources = {['Input_' methods{index}],['Reset_' methods{index}]};
    if index==1
        sources = [sources {'FE_Center_Frequency','FE_Damping_Gain','FE_Sample_Period'}];
    end
    assert(numel(ports.Inport)==numel(sources) && numel(ports.Outport)==2, ...
        'SOGI:LayoutInterface','Unexpected ports on %s.',methodBlocks{index});
    for port = 1:numel(sources)
        position = get_param(ports.Inport(port),'Position');
        place(model,sources{port},[2220 position(2)-25 2520 position(2)+25]);
    end
    channels = {'D','Q'};
    for port = 1:2
        position = get_param(ports.Outport(port),'Position');
        name = sprintf('Meas_SOGI_%s%s_PU_f32',methods{index},channels{port});
        place(model,name,[4750 position(2)-10 4780 position(2)+10]);
    end
end
note(model,'SOGI_FE_INPUT_ROOT',[2170 -530],sprintf([ ...
    'FE physical-parameter input trial | validation status: NOT_RUN\n', ...
    'Input -> reset conditioning -> filter calculation -> D / Q outputs.\n', ...
    'FE receives f0 [Hz], k [1] and Ts [s] through explicit input ports.\n', ...
    'The parameter sources belong to the calling model. ', ...
    'Ts must equal the compiled execution period.\n', ...
    'BE, Tustin and ZOH remain separate comparison branches.']));
annotations = find_system(model,'FindAll','on','SearchDepth',1,'Type','annotation');
for index = 1:numel(annotations)
    if strcmp(get_param(annotations(index),'Tag'),'SOGI_TOP_HEADER')
        object = get_param(annotations(index),'Object');
        object.Text = sprintf(['SOGI discretization comparison\n', ...
            'FE: physical input ports. BE/Tustin/ZOH: legacy dictionary coefficients.\n', ...
            'New FE numerical acceptance: NOT_RUN.']);
        object.Position = [180 -1750];
    end
    if strcmp(get_param(annotations(index),'Tag'),'SOGI_PARAMETERS')
        object = get_param(annotations(index),'Object');
        % Keep the refresh helper's full saved/pending values and symbols.
        object.Position = [180 -1370];
        object.FontName = 'Arial'; object.FontSize = 14;
    end
end
set_param(model,'ZoomFactor','100');
routeScope(model);
end

function scope = scopeByName(root,name)
blocks = find_system(root,'SearchDepth',1,'Type','block','Name',name);
assert(numel(blocks)==1,'SOGI:LayoutBlock', ...
    'Expected exactly one %s in %s.',name,root);
scope = Simulink.ID.getFullName(Simulink.ID.getSID(blocks{1}));
end

function block = place(scope,name,position)
blocks = find_system(scope,'FindAll','on','SearchDepth',1,'Type','block','Name',name);
assert(numel(blocks)==1,'SOGI:LayoutBlock', ...
    'Expected exactly one block named %s in %s.',name,scope);
block = blocks(1);
set_param(block,'Orientation','right','Position',position);
end

function styleScope(scope)
blocks = find_system(scope,'FindAll','on','SearchDepth',1,'Type','block');
blocks(blocks==get_param(scope,'Handle')) = [];
for index = 1:numel(blocks)
    block = blocks(index);
    set_param(block,'FontName','Arial','FontSize','14');
    if any(strcmp(get_param(block,'BlockType'),{'From','Goto'}))
        set_param(block,'ShowName','off');
    else
        set_param(block,'ShowName','on');
    end
end
annotations = find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
for index = 1:numel(annotations)
    object = get_param(annotations(index),'Object');
    object.FontName = 'Arial'; object.FontSize = 14;
end
end

function note(scope,tag,position,text)
handles = find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
match = [];
for index = 1:numel(handles)
    if strcmp(get_param(handles(index),'Tag'),tag)
        match = handles(index);
        break
    end
end
if isempty(match)
    object = Simulink.Annotation(scope,text);
else
    object = get_param(match,'Object');
    object.Text = text;
end
object.Tag = tag;
object.Position = position;
object.FontName = 'Arial'; object.FontSize = 14;
end

function routeScope(scope)
blocks = find_system(scope,'FindAll','on','SearchDepth',1,'Type','block');
blocks(blocks==get_param(scope,'Handle')) = [];
rectangles = zeros(numel(blocks),4);
labels = zeros(0,4);
for index = 1:numel(blocks)
    position = get_param(blocks(index),'Position');
    rectangles(index,:) = position+[-12 -12 12 12];
    if strcmp(get_param(blocks(index),'ShowName'),'on')
        name = get_param(blocks(index),'Name');
        width = max(60,8.7*numel(name));
        center = (position(1)+position(3))/2;
        if strcmp(get_param(blocks(index),'NamePlacement'),'alternate')
            labels(end+1,:) = [center-width/2 position(2)-34 center+width/2 position(2)-2]; %#ok<AGROW>
        else
            labels(end+1,:) = [center-width/2 position(4)+2 center+width/2 position(4)+34]; %#ok<AGROW>
        end
    end
end
lines = find_system(scope,'FindAll','on','SearchDepth',1,'Type','line');
for index = 1:numel(lines)
    line = lines(index);
    source = get_param(line,'SrcPortHandle');
    destination = get_param(line,'DstPortHandle');
    assert(source>0 && numel(destination)==1 && destination>0, ...
        'SOGI:LayoutConnection', ...
        'Layout requires complete single-destination nets in %s; repair topology first.',scope);
    start = double(get_param(source,'Position'));
    finish = double(get_param(destination,'Position'));
    sourceBlock = get_param(line,'SrcBlockHandle');
    destinationBlock = get_param(line,'DstBlockHandle');
    obstacles = [rectangles(blocks~=sourceBlock & blocks~=destinationBlock,:);labels];
    points = orthogonalRoute(start,finish,obstacles);
    set_param(line,'Points',points);
end
end

function best = orthogonalRoute(start,finish,obstacles)
candidates = {[start;finish]};
x = unique([start(1)+65;finish(1)-65;(start(1)+finish(1))/2; ...
    obstacles(:,1)-45;obstacles(:,3)+45]);
if finish(1)>start(1)+40
    x = x(x>start(1)+20 & x<finish(1)-20);
end
for index = 1:numel(x)
    candidates{end+1} = [start;x(index) start(2);x(index) finish(2);finish]; %#ok<AGROW>
end
y = unique([start(2);finish(2);obstacles(:,2)-50;obstacles(:,4)+50]);
for index = 1:numel(y)
    candidates{end+1} = [start;start(1)+65 start(2);start(1)+65 y(index); ...
        finish(1)-65 y(index);finish(1)-65 finish(2);finish]; %#ok<AGROW>
end
best = []; cost = Inf;
for index = 1:numel(candidates)
    points = candidates{index};
    points = points([true;any(diff(points,1,1)~=0,2)],:);
    if clearRoute(points,obstacles)
        candidateCost = sum(abs(diff(points,1,1)),'all')+25*size(points,1);
        if candidateCost<cost
            best = points; cost = candidateCost;
        end
    end
end
assert(~isempty(best),'SOGI:LayoutRoute', ...
    'No unobstructed orthogonal route from [%g %g] to [%g %g].',start,finish);
end

function clear = clearRoute(points,obstacles)
clear = true;
for index = 1:size(points,1)-1
    first = points(index,:); last = points(index+1,:);
    if first(1)==last(1)
        hit = first(1)>obstacles(:,1) & first(1)<obstacles(:,3) & ...
            max(first(2),last(2))>obstacles(:,2) & min(first(2),last(2))<obstacles(:,4);
    elseif first(2)==last(2)
        hit = first(2)>obstacles(:,2) & first(2)<obstacles(:,4) & ...
            max(first(1),last(1))>obstacles(:,1) & min(first(1),last(1))<obstacles(:,3);
    else
        clear = false;
        return
    end
    if any(hit)
        clear = false;
        return
    end
end
end
