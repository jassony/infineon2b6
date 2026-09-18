function sogi_apply_fe_validation_layout
%SOGI_APPLY_FE_VALIDATION_LAYOUT Lay out validation with five DUT inputs.
%   Display and existing line Points only. No topology edits or model saves.

scope = 'sogi_validation';
assert(bdIsLoaded(scope),'SOGI:LayoutModel','Load %s before layout.',scope);
positions = {
    'Input_Sine',[50 160 160 210];
    'Sample_Hold',[270 160 380 210];
    'Publish_Held_Input',[510 170 690 200];
    'Read_Held_Input_0',[710 410 865 440];
    'Input_Single',[900 400 1040 450];
    'Reset_Off',[840 650 1060 700];
    'Four_Methods',[1190 340 2090 940];
    'Method_Outputs',[2450 335 2485 825];
    'Output_Double',[2610 550 2750 610];
    'Publish_Discrete_DQ',[2870 565 3060 595];
    'Read_Discrete_DQ_0',[3080 160 3250 190];
    'Discrete_DQ',[3480 160 3540 190];
    'Read_Held_Input_1',[710 1000 900 1030];
    'Continuous_Reference',[1010 950 1310 1090];
    'Reference_Sampled',[1420 995 1590 1045];
    'Publish_Reference_DQ',[1710 1005 1910 1035];
    'Read_Reference_DQ_0',[3080 300 3250 330];
    'Continuous_DQ',[3480 300 3540 330];
    'Read_Reference_DQ_1',[3050 1235 3210 1265];
    'Read_Reference_DQ_2',[3050 1290 3210 1320];
    'Read_Reference_DQ_3',[3050 1345 3210 1375];
    'Read_Reference_DQ_4',[3050 1400 3210 1430];
    'Repeat_Reference',[3360 1230 3395 1430];
    'Read_Discrete_DQ_1',[3200 1110 3370 1140];
    'Sampled_Error',[3550 1280 3610 1380];
    'Error_DQ',[3810 1315 3870 1345];
    'Read_Discrete_DQ_2',[3080 600 3250 630];
    'Read_Reference_DQ_5',[3080 710 3250 740];
    'Compare_Waveforms',[3800 580 4020 780]};
for index = 1:size(positions,1)
    set_param(findBlock(scope,positions{index,1}), ...
        'Position',positions{index,2},'Orientation','right');
end

dut = findBlock(scope,'Four_Methods');
ports = get_param(dut,'PortHandles');
assert(numel(ports.Inport)==5 && numel(ports.Outport)==8, ...
    'SOGI:LayoutInterface','Four_Methods must have u/reset/f0/k/Ts inputs and 8 outputs.');
sources = {'Input_Single','Reset_Off','FE_Center_Frequency_Hz', ...
    'FE_Damping_Gain','FE_Sample_Period_s'};
for index = 1:numel(sources)
    point = get_param(ports.Inport(index),'Position');
    rectangle = [840 point(2)-25 1060 point(2)+25];
    if index==1
        rectangle = [900 point(2)-25 1040 point(2)+25];
        set_param(findBlock(scope,'Read_Held_Input_0'), ...
            'Position',[710 point(2)-15 865 point(2)+15]);
    end
    set_param(findBlock(scope,sources{index}),'Position',rectangle,'Orientation','right');
end

% Match the eight scalar readout rows to the actual Model Reference ports.
% This keeps parallel forward wires separate after the reference is resized.
mux = findBlock(scope,'Method_Outputs');
muxPorts = get_param(mux,'PortHandles');
assert(numel(muxPorts.Inport)==8,'SOGI:LayoutInterface','Expected an eight-input output packer.');
first = get_param(ports.Outport(1),'Position');
last = get_param(ports.Outport(end),'Position');
muxFirst = get_param(muxPorts.Inport(1),'Position');
muxLast = get_param(muxPorts.Inport(end),'Position');
rectangle = get_param(mux,'Position');
scale = (last(2)-first(2))/(muxLast(2)-muxFirst(2));
top = first(2)-(muxFirst(2)-rectangle(2))*scale;
height = (rectangle(4)-rectangle(2))*scale;
set_param(mux,'Position',[2450 round(top) 2485 round(top+height)]);

blocks = find_system(scope,'FindAll','on','SearchDepth',1,'Type','block');
blocks(blocks==get_param(scope,'Handle')) = [];
for index = 1:numel(blocks)
    set_param(blocks(index),'FontName','Arial','FontSize','14');
    if any(strcmp(get_param(blocks(index),'BlockType'),{'From','Goto'}))
        set_param(blocks(index),'ShowName','off');
    end
end
annotations = find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
for index = 1:numel(annotations)
    object = get_param(annotations(index),'Object');
    object.FontName = 'Arial'; object.FontSize = 14;
    if strcmp(get_param(annotations(index),'Tag'),'SOGI_VALIDATION_HEADER')
        object.Position = [50 20];
        object.Text = sprintf([ ...
            'SOGI VALIDATION | five-input comparison interface\n', ...
            'Left to right: stimulus -> preprocessing -> algorithm/reference ', ...
            '-> postprocessing/error -> outputs.\n', ...
            'FE receives u, reset, f0, k and Ts. ', ...
            'Caller-owned parameter sources configure the FE instance.']);
    end
end
set_param(dut,'ContentPreviewEnabled','off');
set_param(findBlock(scope,'Continuous_Reference'),'ContentPreviewEnabled','off');
set_param(scope,'ZoomFactor','100');
routeScope(scope);
end

function block = findBlock(scope,name)
blocks = find_system(scope,'FindAll','on','SearchDepth',1,'Type','block','Name',name);
assert(numel(blocks)==1,'SOGI:LayoutBlock','Expected one %s in %s.',name,scope);
block = blocks(1);
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
