function snapshot = sogi_capture_port_layout(label)
%SOGI_CAPTURE_PORT_LAYOUT Read geometry, rendered text metrics and semantics.
% No simulation, compilation, parameter writes or model edits.
root=fileparts(mfilename('fullpath'));
out=fullfile(root,'reports','port_aware_layout');
scopes={'sogi_discrete_comparison', ...
    'sogi_discrete_comparison/SOGI_DualOutput_Filter_FE', ...
    'sogi_discrete_comparison/BE','sogi_discrete_comparison/Tustin', ...
    'sogi_discrete_comparison/ZOH','sogi_validation', ...
    'sogi_validation/Continuous_Reference'};
font=java.awt.Font('Arial',0,14);
context=java.awt.font.FontRenderContext(java.awt.geom.AffineTransform(),true,true);
snapshot=struct('scopes',[],'textMetric', ...
    'Arial 14 model-coordinate metrics; calibrated against existing Simulink annotation bounds (within 4 units). Width includes 4 units allowance.');
for si=1:numel(scopes)
    scope=scopes{si};
    assert(bdIsLoaded(bdroot(scope)) && strcmp(get_param(bdroot(scope),'SimulationStatus'),'stopped'));
    blocks=find_system(scope,'FindAll','on','SearchDepth',1,'Type','block');
    blocks(blocks==get_param(scope,'Handle'))=[];
    record=struct('path',scope,'blocks',[],'lines',[],'annotations',[]);
    for bi=1:numel(blocks)
        b=blocks(bi); type=get_param(b,'BlockType'); ph=get_param(b,'PortHandles');
        entry=struct('sid',Simulink.ID.getSID(b),'name',get_param(b,'Name'),'type',type, ...
            'position',get_param(b,'Position'),'fontSize',get_param(b,'FontSize'), ...
            'fontName',get_param(b,'FontName'),'showName',get_param(b,'ShowName'), ...
            'namePlacement',get_param(b,'NamePlacement'),'orientation',get_param(b,'Orientation'), ...
            'group','','minW',0,'minH',0,'pitch',20,'textWidth',0,'textHeight',0, ...
            'insideLabels',struct('left',[],'right',[]),'ports',struct(),'dialog',struct());
        [entry.textWidth,entry.textHeight]=measure(entry.name);
        connectivity=get_param(b,'PortConnectivity');
        for side=fieldnames(ph)'
            pp=ph.(side{1}); values=struct('number',{},'position',{},'connectionPosition',{});
            for pi=1:numel(pp)
                ci=pi;
                if strcmp(side{1},'Outport'),ci=numel(ph.Inport)+pi;end
                cp=get_param(pp(pi),'Position');
                if any(strcmp(side{1},{'Inport','Outport'})),cp=connectivity(ci).Position;end
                values(end+1)=struct('number',get_param(pp(pi),'PortNumber'), ...
                    'position',get_param(pp(pi),'Position'),'connectionPosition',cp); %#ok<AGROW>
            end
            entry.ports.(side{1})=values;
        end
        dp=fieldnames(get_param(b,'DialogParameters'));
        for di=1:numel(dp),entry.dialog.(dp{di})=get_param(b,dp{di});end
        entry.iconText='';entry.iconTextWidth=0;entry.iconTextHeight=0;
        if any(strcmp(type,{'Goto','From'})),entry.iconText=entry.dialog.GotoTag;
        elseif strcmp(type,'Constant'),entry.iconText=entry.dialog.Value;
        elseif strcmp(type,'ModelReference'),entry.iconText=entry.dialog.ModelName;
        end
        if ~isempty(entry.iconText),[entry.iconTextWidth,entry.iconTextHeight]=measure(entry.iconText);end
        n=numel(ph.Inport); m=numel(ph.Outport);
        entry.group=sprintf('%s_%d_%d_%s',type,n,m,entry.orientation);
        switch type
            case {'Inport','Outport'},entry.minW=30;entry.minH=20;
            case {'From','Goto'},entry.minW=100;entry.minH=30;
            case 'UnitDelay',entry.minW=70;entry.minH=40;
            case 'Sum',entry.minW=40;entry.minH=40;
            case 'Mux',entry.minW=10;entry.minH=20;
            case 'Switch',entry.minW=120;entry.minH=60;
            case 'SubSystem'
                entry.minW=200;entry.minH=80;entry.pitch=40;
                if strcmp(scope,'sogi_discrete_comparison'),entry.minW=300;entry.minH=170;end
            case 'ModelReference',entry.minW=300;entry.minH=120;entry.pitch=40;
            case 'Integrator',entry.minW=70;entry.minH=40;
            case 'Scope',entry.minW=120;entry.minH=60;
            case 'ZeroOrderHold',entry.minW=110;entry.minH=40;
            otherwise,entry.minW=120;entry.minH=40;
        end
        if any(strcmp(type,{'SubSystem','ModelReference'}))
            child=getfullname(b);
            if strcmp(type,'ModelReference'),child=get_param(b,'ModelName');end
            for pair={{'Inport','left'},{'Outport','right'}}
                pc=pair{1}; list=find_system(child,'SearchDepth',1,'BlockType',pc{1});
                ports=struct('text',{},'width',{},'height',{});
                for pi=1:numel(list)
                    index=str2double(get_param(list{pi},'Port')); name=get_param(list{pi},'Name');
                    [w,h]=measure(name);ports(index)=struct('text',name,'width',w,'height',h);
                end
                entry.insideLabels.(pc{2})=ports;
            end
        end
        if strcmp(type,'Constant') && startsWith(get_param(b,'Value'),'Cal_')
            entry.group=[entry.group '_PhysicalParameter'];
        end
        record.blocks=[record.blocks entry]; %#ok<AGROW>
    end
    lines=find_system(scope,'FindAll','on','SearchDepth',1,'Type','line');
    for li=1:numel(lines)
        l=lines(li);s=get_param(l,'SrcPortHandle');d=get_param(l,'DstPortHandle');
        assert(s>0 && numel(d)==1 && d>0,'SOGI:LayoutNet','Expected existing complete single-target nets.');
        [labelWidth,labelHeight]=measure(get_param(l,'Name'));
        record.lines=[record.lines struct('sourceSid',Simulink.ID.getSID(get_param(l,'SrcBlockHandle')), ...
            'sourcePort',get_param(s,'PortNumber'),'destinationSid',Simulink.ID.getSID(get_param(l,'DstBlockHandle')), ...
            'destinationPort',get_param(d,'PortNumber'),'name',get_param(l,'Name'),'points',get_param(l,'Points'), ...
            'labelWidth',labelWidth,'labelHeight',labelHeight)]; %#ok<AGROW>
    end
    annotations=find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
    for ai=1:numel(annotations)
        a=get_param(annotations(ai),'Object');
        record.annotations=[record.annotations struct('tag',a.Tag,'text',a.Text,'position',a.Position,'fontSize',a.FontSize)]; %#ok<AGROW>
    end
    snapshot.scopes=[snapshot.scopes record]; %#ok<AGROW>
end
sogi_write_review_file(fullfile(out,[char(label) '.json']),unicode2native(jsonencode(snapshot),'UTF-8'));
    function [w,h]=measure(str)
        rows=splitlines(string(str)); w=0;h=0;
        for ti=1:numel(rows)
            rect=font.getStringBounds(char(rows(ti)),context);
            w=max(w,rect.getWidth());h=h+rect.getHeight();
        end
        w=ceil(w+4);h=ceil(h+2);
    end
end
